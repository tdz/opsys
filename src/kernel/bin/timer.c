/*
 *  opsys - A small, experimental operating system
 *  Copyright (C) 2016  Thomas Zimmermann
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "timer.h"
#include <assert.h>
#include <stddef.h>
#include "interupt.h"

static struct alarm*
alarm_of_list(struct list* item)
{
    assert(item);
    return containerof(item, struct alarm, timer_entry);
}

int
alarm_init(struct alarm* alarm, timeout_t (*func)(struct alarm*))
{
    assert(alarm);
    assert(func);

    list_init_item(&alarm->timer_entry);
    alarm->timestamp_ns = 0;
    alarm->func = func;

    return 0;
}

bool
alarm_has_expired(const struct alarm* alarm, timestamp_t timestamp_ns)
{
    assert(alarm);
    return alarm->timestamp_ns <= timestamp_ns;
}

struct timer {
    struct timer_drv* drv;
    /* timestamp of when the timer last fired */
    timestamp_t timestamp_ns;
    struct list alarm_list;
};

static struct timer g_timer;

int
init_timer(struct timer_drv* drv)
{
    assert(drv);

    g_timer.drv = drv;
    g_timer.timestamp_ns = 0;
    list_init_head(&g_timer.alarm_list);

    return 0;
}

void
uninit_timer()
{ }

static int
cmp_timestamp(struct list* newitem, struct list* item)
{
    const struct alarm* lhs = alarm_of_list(newitem);
    const struct alarm* rhs = alarm_of_list(item);

    long long diff = lhs->timestamp_ns - rhs->timestamp_ns;

    return (diff > 0) - (diff < 0);
}

void
handle_timeout(timestamp_t timestamp_ns)
{
    /* We fetch items from the list of alarms and loop while
     * the timestamp is in the past. This way, we handle all
     * alarms that have expired. */

    struct list* item = list_begin(&g_timer.alarm_list);

    while (item != list_end(&g_timer.alarm_list)) {

        struct alarm* alarm = alarm_of_list(item);

        if (!alarm_has_expired(alarm, timestamp_ns)) {
            /* We've handled all expired alarms. */
            break;
        }

        struct list* next = list_next(item);
        list_dequeue(item);

        timeout_t reltime_ns = alarm->func(alarm);

        /* For periodic alarms, We now update the alarm's timestamp
         * and re-enqueue it at the correct position. If the alarm is
         * non-periodic, we guarantee to not touch the alarm structure
         * after the callback has returned. */

        if (reltime_ns) {
            alarm->timestamp_ns = timestamp_ns + reltime_ns;
            list_enqueue_sorted(&g_timer.alarm_list,
                                &alarm->timer_entry,
                                cmp_timestamp);
        }

        item = next;
    }

    if (item != list_end(&g_timer.alarm_list)) {
        const struct alarm* alarm = alarm_of_list(item);
        timeout_t timeout_ns = alarm->timestamp_ns - timestamp_ns;
        timer_drv_set_timeout(g_timer.drv, timeout_ns);
    } else {
        timer_drv_clear_timeout(g_timer.drv);
    }
}

int
timer_add_alarm(struct alarm* alarm, timeout_t reltime_ns)
{
    assert(alarm);

    bool ints_on = cli_if_on();

    if (list_is_empty(&g_timer.alarm_list)) {
        timer_drv_set_timeout(g_timer.drv, reltime_ns);
    }

    alarm->timestamp_ns = g_timer.timestamp_ns + reltime_ns;
    list_enqueue_sorted(&g_timer.alarm_list,
                        &alarm->timer_entry,
                        cmp_timestamp);

    sti_if_on(ints_on);

    return 0;
}

void
timer_remove_alarm(struct alarm* alarm)
{
    assert(alarm);

    bool ints_on = cli_if_on();

    list_dequeue(&alarm->timer_entry);

    if (list_is_empty(&g_timer.alarm_list)) {
        timer_drv_clear_timeout(g_timer.drv);
    }

    sti_if_on(ints_on);
}
