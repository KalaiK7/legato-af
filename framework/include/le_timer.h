/**
 * @page c_timer Timer API
 *
 * @ref le_timer.h "API Reference"
 *
 * <HR>
 *
 * This module provides an API for managing and using timers.
 *
 * @note
 * This is an initial version of the API that only provides support for relative timers (e.g., expires
 * in 10 seconds).  Absolute timers allow a specific time/date to be used, and will be supported in a
 * future version of this API.
 *
 *
 * @section timer_objects Creating/Deleting Timer Objects
 *
 * Timers are created using @ref le_timer_Create. The timer name is used for logging purposes only.
 *
 * The following attributes of the timer can be set:
 *  - @ref le_timer_SetHandler
 *  - @ref le_timer_SetInterval
 *  - @ref le_timer_SetRepeat
 *  - @ref le_timer_SetContextPtr
 *
 * The repeat count defaults to 1, so that the timer is initially a one-shot timer. All the other
 * attributes must be explicitly set.  At a minimum, the interval must be set before the timer can be
 * used.  Note that these attributes can only be set if the timer is not currently running; otherwise,
 * an error will be returned.
 *
 * Timers must be explicitly deleted using @ref le_timer_Delete. If the timer is currently running,
 * it'll be stopped before being deleted. If a timer uses @ref le_timer_SetContextPtr, and the
 * context pointer is allocated memory, then the context pointer must be freed when deleting the timer.
 * The following function can be used for this:
 * @code
 * static void DeleteTimerAndFreePtr(le_timer_Ref_t t)
 * {
 *     le_timer_Stop( t );
 *     free( le_timer_GetContextPtr( t ) );
 *     le_timer_Delete( t );  // timer ref is now invalid
 * }
 * @endcode
 * You can call this function anywhere, including in the timer handler.
 *
 * @section timer_usage Using Timers
 *
 * A timer is started using @ref le_timer_Start. If it's already running, then it won't be
 * modified; instead an error will be returned. To restart a currently running timer, use @ref
 * le_timer_Restart.
 *
 * A timer is stopped using @ref le_timer_Stop.  If it's not currently running, an error
 * will be returned, and nothing more will be done.
 *
 * To determine if the timer is currently running, use @ref le_timer_IsRunning.
 *
 * When a timer expires, if the timer expiry handler is set by @ref le_timer_SetHandler, the
 * handler will be called with a reference to the expired timer. If additional data is required in the
 * handler, @ref le_timer_SetContextPtr can be used to set the appropriate context before starting the
 * timer, and @ref le_timer_GetContextPtr can be used to retrieve the context while in the handler.
 * In addition, a suspended system will also wake up by default if the timer expires. If this behaviour
 * is not desired, user can disable the wake up by passing false into @ref le_timer_SetWakeup.
 *
 * The number of times that a timer has expired can be retrieved by @ref le_timer_GetExpiryCount. This
 * count is independent of whether there is an expiry handler for the timer.
 *
 * @section le_timer_thread Thread Support
 *
 * A timer should only be used by the thread that created it. It's not safe for a thread to use
 * or manipulate a timer that belongs to another thread. The timer expiry handler is called by the
 * event loop of the thread that starts the timer.
 *
 * The call to the timer expiry handler may not occur immediately after the timer expires, depending
 * on which other functions are called from the event loop. The amount of delay is entirely
 * dependent on other work done in the event loop. For a repeating timer, if this delay is longer
 * than the timer period, one or more timer expiries may be dropped. To reduce the likelihood of
 * dropped expiries, the combined execution time of all handlers called from the event loop should
 *  ideally be less than the timer period.
 *
 * See @ref c_eventLoop for details on running the event loop of a thread.
 *
 * @section le_timer_suspend Suspend Support
 *
 * The timer runs even when system is suspended. <br>
 * If the timer expires while the system is suspended, it will wake up the system.
 *
 * @section timer_errors Fatal Errors
 *
 * The process will exit under any of the following conditions:
 *  - If an invalid timer object is given to:
 *     - @ref le_timer_Delete
 *     - @ref le_timer_SetHandler
 *     - @ref le_timer_SetInterval
 *     - @ref le_timer_SetRepeat
 *     - @ref le_timer_Start
 *     - @ref le_timer_Stop
 *     - @ref le_timer_Restart
 *     - @ref le_timer_SetContextPtr
 *     - @ref le_timer_GetContextPtr
 *     - @ref le_timer_GetExpiryCount
 *     - @ref le_timer_SetWakeup
 *
 * @section timer_troubleshooting Troubleshooting
 *
 * Timers can be traced by enabling the log trace keyword "timers" in the "framework" component.
 *
 * See @ref c_log_controlling for more information.
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

//--------------------------------------------------------------------------------------------------
/**
 * @file le_timer.h
 *
 * Legato @ref c_timer include file.
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */
//--------------------------------------------------------------------------------------------------

#ifndef LEGATO_TIMER_INCLUDE_GUARD
#define LEGATO_TIMER_INCLUDE_GUARD


//--------------------------------------------------------------------------------------------------
/**
 * Timer object.  Created by le_timer_Create().
 */
//--------------------------------------------------------------------------------------------------
typedef struct le_timer* le_timer_Ref_t;


//--------------------------------------------------------------------------------------------------
/**
 * Prototype for timer expiry handler function.
 *
 * @param
 *      timerRef Timer that has expired
 */
//--------------------------------------------------------------------------------------------------
typedef void (*le_timer_ExpiryHandler_t)
(
    le_timer_Ref_t timerRef
);


//--------------------------------------------------------------------------------------------------
/**
 * Create the timer object.
 *
 * @return
 *      A reference to the timer object.
 */
//--------------------------------------------------------------------------------------------------
le_timer_Ref_t le_timer_Create
(
    const char* nameStr                 ///< [IN]  Name of the timer.
);


//--------------------------------------------------------------------------------------------------
/**
 * Delete the timer object.
 *
 * @note
 *      If an invalid timer object is given, the process exits.
 */
//--------------------------------------------------------------------------------------------------
void le_timer_Delete
(
    le_timer_Ref_t timerRef                 ///< [IN] Delete this timer object
);


//--------------------------------------------------------------------------------------------------
/**
 * Set the timer expiry handler function.
 *
 * If the handler is NULL, then the previous handler will be removed.
 *
 * @return
 *      - LE_OK on success
 *      - LE_BUSY if the timer is currently running
 *
 * @note
 *      If an invalid timer object is given, the process exits.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_timer_SetHandler
(
    le_timer_Ref_t timerRef,                ///< [IN] Set expiry handler for this timer object.
    le_timer_ExpiryHandler_t handlerFunc    ///< [IN] Handler function to call on expiry.
);


//--------------------------------------------------------------------------------------------------
/**
 * Set the timer interval.
 *
 * Timer will expire after the interval has elapsed.
 *
 * @return
 *      - LE_OK on success
 *      - LE_BUSY if the timer is currently running
 *
 * @note
 *      If an invalid timer object is given, the process exits.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_timer_SetInterval
(
    le_timer_Ref_t timerRef,     ///< [IN] Set interval for this timer object.
    le_clk_Time_t interval       ///< [IN] Timer interval.
);


//--------------------------------------------------------------------------------------------------
/**
 * Set the timer interval using milliseconds.
 *
 * Timer will expire after the interval has elapsed.
 *
 * @return
 *      - LE_OK on success
 *      - LE_BUSY if the timer is currently running
 *
 * @note
 *      If an invalid timer object is given, the process exits.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_timer_SetMsInterval
(
    le_timer_Ref_t timerRef,     ///< [IN] Set interval for this timer object.
    uint32_t interval            ///< [IN] Timer interval in milliseconds.
);


//--------------------------------------------------------------------------------------------------
/**
 * Set how many times the timer will repeat.
 *
 * Timer will repeat the given number of times.  A value of 0 means repeat indefinitely.
 * The default is 1, so that a one-shot timer is the default.
 *
 * @return
 *      - LE_OK on success
 *      - LE_BUSY if the timer is currently running
 *
 * @note
 *      If an invalid timer object is given, the process exits.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_timer_SetRepeat
(
    le_timer_Ref_t timerRef,     ///< [IN] Set repeat count for this timer object
    uint32_t repeatCount         ///< [IN] Number of times the timer will repeat (0 = forever).
);


//--------------------------------------------------------------------------------------------------
/**
 * Configure if timer expiry will wake up a suspended system.
 *
 * @return
 *      - LE_OK on success
 *      - LE_BUSY if the timer is currently running
 *
 * @note
 *      The default timer expiry behaviour will wake up the system.
 *      If an invalid timer object is given, the process exits.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_timer_SetWakeup
(
    le_timer_Ref_t timerRef,     ///< [IN] Disable system wake up for this timer object
    bool wakeupEnabled           ///< [IN] Flag to determine timer will wakeup or not
);


//--------------------------------------------------------------------------------------------------
/**
 * Set context pointer for the timer.
 *
 * This can be used to pass data to the timer when it expires.
 *
 * @return
 *      - LE_OK on success
 *      - LE_BUSY if the timer is currently running
 *
 * @note
 *      If an invalid timer object is given, the process exits.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_timer_SetContextPtr
(
    le_timer_Ref_t timerRef,     ///< [IN] Set context pointer for this timer object
    void* contextPtr             ///< [IN] Context Pointer.
);


//--------------------------------------------------------------------------------------------------
/**
 * Get context pointer for the timer.
 *
 * This can be used when the timer expires to retrieve data that was previously set.
 *
 * @return
 *      Context pointer, which could be NULL if it was not set.
 *
 * @note
 *      If an invalid timer object is given, the process exits.
 */
//--------------------------------------------------------------------------------------------------
void* le_timer_GetContextPtr
(
    le_timer_Ref_t timerRef      ///< [IN] Get context pointer for this timer object
);


//--------------------------------------------------------------------------------------------------
/**
 * Get the expiry count of a timer.
 *
 * The count is returned for  currently running and idle timers.  The expiry count
 * is reset every time the timer is (re)started.
 *
 * @return
 *      Expiry count, or zero if the timer has never expired.
 *
 * @note
 *      If an invalid timer object is given, the process exits.
 */
//--------------------------------------------------------------------------------------------------
uint32_t le_timer_GetExpiryCount
(
    le_timer_Ref_t timerRef      ///< [IN] Get expiry count for this timer object
);


//--------------------------------------------------------------------------------------------------
/**
 * Start the timer.
 *
 * Start the given timer. The timer must not be currently running.
 *
 * @return
 *      - LE_OK on success
 *      - LE_BUSY if the timer is already running
 *
 * @note
 *      If an invalid timer object is given, the process exits.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_timer_Start
(
    le_timer_Ref_t timerRef      ///< [IN] Start this timer object.
);


//--------------------------------------------------------------------------------------------------
/**
 * Stop the timer.
 *
 * Stop the given timer. The timer must be running.
 *
 * @return
 *      - LE_OK on success
 *      - LE_FAULT if the timer is not currently running
 *
 * @note
 *      If an invalid timer object is given, the process exits.
 */
//--------------------------------------------------------------------------------------------------
le_result_t le_timer_Stop
(
    le_timer_Ref_t timerRef      ///< [IN] Stop this timer object.
);


//--------------------------------------------------------------------------------------------------
/**
 * Re-start the timer.
 *
 * Start the given timer. If the timer is currently running, it will be stopped and then started.
 * If the timer is not currently running, it will be started.
 *
 * @note
 *      If an invalid timer object is given, the process exits.
 */
//--------------------------------------------------------------------------------------------------
void le_timer_Restart
(
    le_timer_Ref_t timerRef      ///< [IN] (Re)start this timer object.
);


//--------------------------------------------------------------------------------------------------
/**
 * Is the timer currently running?
 *
 * @note
 *      If an invalid timer object is given, the process exits.
 */
//--------------------------------------------------------------------------------------------------
bool le_timer_IsRunning
(
    le_timer_Ref_t timerRef      ///< [IN] Is this timer object currently running?
);


#endif // LEGATO_TIMER_INCLUDE_GUARD

