// #############################################################################
// #### Copyright ##############################################################
// #############################################################################

/*
 * Copyright 2024 BaSSeM
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

// #############################################################################
// #### Description ############################################################
// #############################################################################

/**
 *  @file
 *
 *  @brief Platform USB STM32L496VGT6P Driver
 */

// #############################################################################
// #### Control Include(s) #####################################################
// #############################################################################

// #############################################################################
// #### Control Macro(s) #######################################################
// #############################################################################

// #############################################################################
// #### File Guard #############################################################
// #############################################################################

/**
 *  @addtogroup Platform_USB_Driver
 *
 *  @{
 */

/**
 *  @defgroup Platform_USB_STM32L496VGT6P STM32L496VGT6P
 *
 *  @{
 */

#ifndef USB_STM32L496VGT6P_H_
    #define USB_STM32L496VGT6P_H_

    #ifdef __cplusplus
extern "C"
{
    #endif /* __cplusplus */

    // #############################################################################
    // #### Include(s) #############################################################
    // #############################################################################

    // #############################################################################
    // #### Public Macro(s) ########################################################
    // #############################################################################

    // #############################################################################
    // #### Public Type(s) #########################################################
    // #############################################################################

    /**
     *  @brief USB STM32L496VGT6P Operation Status
     *
     *  @enum USB_STM32L496VGT6P_Status_t
     */
    typedef enum USB_STM32L496VGT6P_Status
    {
        USB_STM32L496VGT6P_Status_Success = 0,
        USB_STM32L496VGT6P_Status_ArgumentInvalid,
        USB_STM32L496VGT6P_Status_NotSupported,
        USB_STM32L496VGT6P_Status_Error,
        USB_STM32L496VGT6P_Status_Busy,
        USB_STM32L496VGT6P_Status_Timeout,
    } USB_STM32L496VGT6P_Status_t;

    /**
     *  @brief USB STM32L496VGT6P
     *
     *  @enum USB_STM32L496VGT6P_t
     */
    typedef enum USB_STM32L496VGT6P
    {
        USB_STM32L496VGT6P_1 = 0, ///< USB 1
        USB_STM32L496VGT6P_Count  ///< Count
    } USB_STM32L496VGT6P_t;

    // TODO Support Transmit & Receive Over/From End-Points Through Callbacks
    /**
     *  @brief USB STM32L496VGT6P End-Point
     *
     *  @enum USB_STM32L496VGT6P_t
     */
    typedef enum USB_STM32L496VGT6P_EndPoint
    {
        USB_STM32L496VGT6P_EndPoint_1 = 0, ///< End-point 1
        USB_STM32L496VGT6P_EndPoint_2,     ///< End-point 2
        USB_STM32L496VGT6P_EndPoint_Count  ///< Count
    } USB_STM32L496VGT6P_EndPoint_t;

    /**
     *  @brief USB STM32L496VGT6P Instance Context
     *
     *  @struct USB_STM32L496VGT6P_Instance_Context_t
     */
    typedef struct USB_STM32L496VGT6P_Instance_Context USB_STM32L496VGT6P_Instance_Context_t;

    /**
     *  @brief USB STM32L496VGT6P Instance
     *
     *  @struct USB_STM32L496VGT6P_Instance_t
     */
    typedef struct USB_STM32L496VGT6P_Instance
    {
        USB_STM32L496VGT6P_t USBx;
        GPIO_t DP;
        GPIO_t DM;
        GPIO_t VBUS_Sense;
        USB_STM32L496VGT6P_Instance_Context_t * Context;
    } USB_STM32L496VGT6P_Instance_t;

    /**
     *  @brief USB STM32L496VGT6P Data
     */
    typedef uint8_t USB_STM32L496VGT6P_Data_t;

    /**
     *  @brief USB STM32L496VGT6P Data Length
     */
    typedef uint32_t USB_STM32L496VGT6P_DataLength_t;

    // #############################################################################
    // #### Public Method(s) #######################################################
    // #############################################################################

    /**
     *  @brief Initializes specified USB STM32L496VGT6P Instance
     *
     *  @param[in] Instance Instance
     *
     *  @return USB_STM32L496VGT6P_Status_t
     */
    USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Initialize( USB_STM32L496VGT6P_Instance_t * Instance );

    /**
     *  @brief Cycles specified USB STM32L496VGT6P Instance
     *
     *  @param[in] Instance Instance
     *
     *  @return USB_STM32L496VGT6P_Status_t
     */
    USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Cycle( USB_STM32L496VGT6P_Instance_t * Instance );

    /**
     *  @brief De-initializes specified USB STM32L496VGT6P Instance
     *
     *  @param[in] Instance Instance
     *
     *  @return USB_STM32L496VGT6P_Status_t
     */
    USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_DeInitialize( USB_STM32L496VGT6P_Instance_t * Instance );

    /**
     *  @brief Writes data to specified USB STM32L496VGT6P Instance
     *
     *  @param[in] Instance   Instance
     *  @param[in] Data       Data buffer
     *  @param[in] DataLength Length of data buffer
     *
     *  @return USB_STM32L496VGT6P_Status_t
     */
    USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Write( USB_STM32L496VGT6P_Instance_t * Instance, USB_STM32L496VGT6P_Data_t * Data, USB_STM32L496VGT6P_DataLength_t DataLength );

    /**
     *  @brief Reads data from specified USB STM32L496VGT6P Instance
     *
     *  @param[in]     Instance   Instance
     *  @param[in,out] Data       Data buffer
     *  @param[in]     DataLength Length of data buffer
     *
     *  @return USB_STM32L496VGT6P_Status_t
     */
    USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Read( USB_STM32L496VGT6P_Instance_t * Instance, USB_STM32L496VGT6P_Data_t * Data, USB_STM32L496VGT6P_DataLength_t DataLength );

    // #############################################################################
    // #### Public Variable(s) #####################################################
    // #############################################################################

    /**
     *  @brief Version
     */
    extern const char USB_STM32L496VGT6P_VERSION[];

    // #############################################################################
    // #### File Guard #############################################################
    // #############################################################################

    #ifdef __cplusplus
} /* extern "C" */
    #endif /* __cplusplus */

#endif /* USB_STM32L496VGT6P_H_ */

/**
 *  @}
 *
 *  @}
 */

// #############################################################################
// #### END OF FILE ############################################################
// #############################################################################
