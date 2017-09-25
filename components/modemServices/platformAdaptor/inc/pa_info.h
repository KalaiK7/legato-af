/**
 * @page c_pa_info Modem Information Platform Adapter API
 *
 * @ref pa_info.h "API Reference"
 *
 * <HR>
 *
 * @section pa_info_toc Table of Contents
 *
 *  - @ref pa_info_intro
 *  - @ref pa_info_rational
 *
 *
 * @section pa_info_intro Introduction
 * These APIs are on the top of the platform-dependent adapter layer. They are independent of the
 * implementation. They guarantee the portability on different kind of platform without any changes
 * for the components developped upon these APIs.
 *
 *
 * @section pa_info_rational Rational
 * These functions are all blocking functions, so that they return when the modem has answered or
 * when a timeout has occured due to an interrupted communication with the modem.
 *
 * They all verify the validity and the range of the input parameters before performing the modem
 * operation.
 *
 * Some functions are used to get some information with a fixed pattern string,
 * in this case no buffer overflow will occur has they always get a fixed string length.
 *
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 */


/** @file pa_info.h
 *
 * Legato @ref c_pa_info include file.
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 */

#ifndef LEGATO_PAINFO_INCLUDE_GUARD
#define LEGATO_PAINFO_INCLUDE_GUARD


#include "legato.h"
#include "interfaces.h"

//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions.
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/**
 * Maximum 'International Mobile Equipment Identity length.
 */
//--------------------------------------------------------------------------------------------------
#define PA_INFO_IMEI_MAX_LEN     LE_INFO_IMEI_MAX_LEN

//--------------------------------------------------------------------------------------------------
/**
 * Maximum 'International Mobile Equipment Identity length.
 */
//--------------------------------------------------------------------------------------------------
#define PA_INFO_IMEI_MAX_BYTES   LE_INFO_IMEI_MAX_BYTES


//--------------------------------------------------------------------------------------------------
/**
 * Maximum number of characters (excluding null terminator) in a device model
 * identification string.
 **/
//--------------------------------------------------------------------------------------------------
#define PA_INFO_DEVICE_MODEL_MAX_LEN     256


//--------------------------------------------------------------------------------------------------
/**
 * Type definition for an 'International Mobile Equipment Identity' (16 digits)
 */
//--------------------------------------------------------------------------------------------------
typedef char pa_info_Imei_t[PA_INFO_IMEI_MAX_BYTES];



//--------------------------------------------------------------------------------------------------
/**
 * Type definition for a 'Device Model ID'.
 */
//--------------------------------------------------------------------------------------------------
typedef char pa_info_DeviceModel_t[PA_INFO_DEVICE_MODEL_MAX_LEN + 1];



//--------------------------------------------------------------------------------------------------
// APIs.
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/**
 * Get the firmware version string
 *
 * @return
 *      - LE_OK on success
 *      - LE_NOT_FOUND if the version string is not available
 *      - LE_OVERFLOW if version string to big to fit in provided buffer
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_info_GetFirmwareVersion
(
    char* versionPtr,        ///< [OUT] Firmware version string
    size_t versionSize       ///< [IN] Size of version buffer
);


//--------------------------------------------------------------------------------------------------
/**
 * Get the bootloader version string
 *
 * @return
 *      - LE_OK on success
 *      - LE_NOT_FOUND if the version string is not available
 *      - LE_OVERFLOW if version string to big to fit in provided buffer
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_info_GetBootloaderVersion
(
    char* versionPtr,        ///< [OUT] Firmware version string
    size_t versionSize       ///< [IN] Size of version buffer
);


//--------------------------------------------------------------------------------------------------
/**
 * This function get the International Mobile Equipment Identity (IMEI).
 *
 * @return
 * - LE_FAULT         The function failed to get the value.
 * - LE_TIMEOUT       No response was received from the Modem.
 * - LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_info_GetImei
(
    pa_info_Imei_t imei   ///< [OUT] IMEI value
);


//--------------------------------------------------------------------------------------------------
/**
 * This function gets the device model identity.
 *
 * @return
 * - LE_FAULT         The function failed to get the value.
 * - LE_OVERFLOW      The device model identity length exceed the maximum length.
 * - LE_OK            The function succeeded.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_info_GetDeviceModel
(
    pa_info_DeviceModel_t model   ///< [OUT] Model string (null-terminated).
);


//--------------------------------------------------------------------------------------------------
/**
 * Get the CDMA device Mobile Equipment Identifier (MEID).
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The device Mobile Equipment identifier length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_info_GetMeid
(
    char* meidStr,           ///< [OUT] Firmware version string
    size_t meidStrSize       ///< [IN] Size of version buffer
);


//--------------------------------------------------------------------------------------------------
/**
 * Get the CDMA Electronic Serial Number (ESN) of the device.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The Electric SerialNumbe length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_info_GetEsn
(
    char* esnStr,
        ///< [OUT]
        ///< The Electronic Serial Number (ESN) of the device
        ///<  string (null-terminated).

    size_t esnStrNumElements
        ///< [IN]
);


//--------------------------------------------------------------------------------------------------
/**
 * Get the CDMA Mobile Identification Number (MIN).
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The CDMA Mobile Identification Number length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_info_GetMin
(
    char        *minStr,    ///< [OUT] The phone Number
    size_t       minStrSize ///< [IN]  Size of phoneNumberStr
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the version of Preferred Roaming List (PRL).
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_NOT_FOUND     The information is not available.
 *      - LE_FAULT         The function failed to get the value.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_info_GetPrlVersion
(
    uint16_t* prlVersionPtr
        ///< [OUT]
        ///< The Preferred Roaming List (PRL) version.
);


///--------------------------------------------------------------------------------------------------
/**
 * Get the CDMA Preferred Roaming List (PRL) only preferences status.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_NOT_FOUND     The information is not available.
 *      - LE_FAULT         The function failed to get the value.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_info_GetPrlOnlyPreference
(
    bool* prlOnlyPreferencePtr      ///< The Cdma PRL only preferences status.
);


//--------------------------------------------------------------------------------------------------
/**
 * Get the CDMA Network Access Identifier (NAI) string in ASCII text.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The Mobile Station ISDN Numbe length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_info_GetNai
(
    char* naiStr,
        ///< [OUT]
        ///< The Network Access Identifier (NAI)
        ///<  string (null-terminated).

    size_t naiStrNumElements
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the Manufacturer Name string in ASCII text.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The Manufacturer Name length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_info_GetManufacturerName
(
    char* mfrNameStr,
        ///< [OUT]
        ///< The Manufacturer Name string (null-terminated).

    size_t mfrNameStrNumElements
        ///< [IN]
);


//--------------------------------------------------------------------------------------------------
/**
 * Get the Product Requirement Information Part Number and Revision Number strings in ASCII text.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The Part or the Revision Number strings length exceed the maximum length.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_info_GetPriId
(
    char* priIdPnStr,
        ///< [OUT]
        ///< The Product Requirement Information Identifier
        ///<  (PRI ID) Part Number string (null-terminated).

    size_t priIdPnStrNumElements,
        ///< [IN]

    char* priIdRevStr,
        ///< [OUT]
        ///< The Product Requirement Information Identifier
        ///<  (PRI ID) Revision Number string (null-terminated).

    size_t priIdRevStrNumElements
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the product stock keeping unit number (SKU) string in ASCII text.
 *
 * @return
 *      - LE_OK            The function succeeded.
 *      - LE_FAULT         The function failed to get the value.
 *      - LE_OVERFLOW      The SKU number string length exceeds the maximum length.
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_info_GetSku
(
    char* skuIdStr,
        ///< [OUT] Product SKU ID string.

    size_t skuIdStrNumElements
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the Platform Serial Number (PSN) string.
 *
 * @return
 *      - LE_OK on success
 *      - LE_OVERFLOW if Platform Serial Number to big to fit in provided buffer
 *      - LE_FAULT for any other errors
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_info_GetPlatformSerialNumber
(
    char* platformSerialNumberStr,
        ///< [OUT]
        ///< Platform Serial Number string.

    size_t platformSerialNumberStrNumElements
        ///< [IN]
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the RF devices working status (i.e. working or broken) of modem's RF devices such as
 * power amplifier, antenna switch and transceiver. That status is updated every time the module
 * power on.
 *
 * @return
 *      - LE_OK on success
 *      - LE_UNSUPPORTED request not supported
 *      - LE_FAULT function failed to get the RF devices working status
 *      - LE_OVERFLOW the number of statuses exceeds the maximum size
 *        (LE_INFO_RF_DEVICES_STATUS_MAX)
 */
//--------------------------------------------------------------------------------------------------
LE_SHARED le_result_t pa_info_GetRfDeviceStatus
(
    uint16_t* manufacturedIdPtr,
        ///< [OUT] Manufactured identifier (MID)

    size_t* manufacturedIdNumElementsPtr,
        ///< [INOUT]

    uint8_t* productIdPtr,
        ///< [OUT] Product identifier (PID)

    size_t* productIdNumElementsPtr,
        ///< [INOUT]

    bool* statusPtr,
        ///< [OUT] Status of the specified device (MID,PID):
        ///<       0 means something wrong in the RF device,
        ///<       1 means no error found in the RF device

    size_t* statusNumElementsPtr
        ///< [INOUT]
);
#endif // LEGATO_PAINFO_INCLUDE_GUARD
