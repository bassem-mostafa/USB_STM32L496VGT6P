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

// #############################################################################
// #### Control Include(s) #####################################################
// #############################################################################

#include "Platform.h"

// #############################################################################
// #### Control Macro(s) #######################################################
// #############################################################################

#ifndef DEBUG
    #define DEBUG
#endif

#ifdef DEBUG
    #undef DEBUG
#endif

// #############################################################################
// #### File Guard #############################################################
// #############################################################################

#ifdef STM32L496xx

// #############################################################################
// #### Include(s) #############################################################
// #############################################################################

    #include "../../USB_Internal.h"
    #include "USB_STM32L496VGT6P.h"

    #include "stm32l4xx.h"
    #include "usbd_def.h"
// #include "usbd_cdc_if.h" // FIXME

// #############################################################################
// #### Private Macro(s) #######################################################
// #############################################################################

    #define USB_STM32L496VGT6P_BUFFER_SIZE_TRANSMIT 0 // APP_TX_DATA_SIZE // FIXME
    #define USB_STM32L496VGT6P_BUFFER_SIZE_RECEIVE  0 // APP_RX_DATA_SIZE  // FIXME

// #############################################################################
// #### Private Type(s) ########################################################
// #############################################################################

typedef enum USB_STM32L496VGT6P_Event
{
    USB_STM32L496VGT6P_Event_None = 0,
    USB_STM32L496VGT6P_Event_TxComplete = UTIL_BIT( 0 ),
    USB_STM32L496VGT6P_Event_RxComplete = UTIL_BIT( 1 ),
    USB_STM32L496VGT6P_Event_AbortTxComplete = UTIL_BIT( 2 ),
    USB_STM32L496VGT6P_Event_AbortRxComplete = UTIL_BIT( 3 ),
    USB_STM32L496VGT6P_Event_AbortComplete = UTIL_BIT( 4 ),
    USB_STM32L496VGT6P_Event_ErrorParity = UTIL_BIT( 5 ),
    USB_STM32L496VGT6P_Event_ErrorNoise = UTIL_BIT( 6 ),
    USB_STM32L496VGT6P_Event_ErrorFrame = UTIL_BIT( 7 ),
    USB_STM32L496VGT6P_Event_ErrorOverrun = UTIL_BIT( 8 ),
    USB_STM32L496VGT6P_Event_ErrorDMA = UTIL_BIT( 9 ),
    USB_STM32L496VGT6P_Event_ErrorReceiverTimeout = UTIL_BIT( 10 ),
    USB_STM32L496VGT6P_Event_RxEvent = UTIL_BIT( 11 ),
} USB_STM32L496VGT6P_Event_t;

typedef struct USB_STM32L496VGT6P_Buffer_Receive
{
    volatile uint32_t Length;
    uint8_t Content[ USB_STM32L496VGT6P_BUFFER_SIZE_RECEIVE ];
} USB_STM32L496VGT6P_Buffer_Receive_t;

typedef struct USB_STM32L496VGT6P_Buffer_Transmit
{
    volatile uint32_t Length;
    uint8_t Content[ USB_STM32L496VGT6P_BUFFER_SIZE_TRANSMIT ];
} USB_STM32L496VGT6P_Buffer_Transmit_t;

typedef struct USB_STM32L496VGT6P_Instance_Context
{
    PCD_HandleTypeDef USBx;
    volatile USB_STM32L496VGT6P_Status_t Status;
    USB_STM32L496VGT6P_Buffer_Receive_t Receive;
    USB_STM32L496VGT6P_Buffer_Transmit_t Transmit;
    USB_STM32L496VGT6P_Event_t Event; // TODO Does it need a context to be associated ? in addition to a queue ?
} USB_STM32L496VGT6P_Instance_Context_t;

typedef struct USB_STM32L496VGT6P_Context
{
    TIM_Timestamp_t Timestamp;
    USB_STM32L496VGT6P_Instance_Context_t Context[ USB_STM32L496VGT6P_Count ];
} USB_STM32L496VGT6P_Context_t;

// #############################################################################
// #### Private Method(s) Prototype ############################################
// #############################################################################

static GPIO_Status_t GPIO_CallbackOnInterrupt( GPIO_t GPIOx, GPIO_ContextOnInterrupt_t Context );

void USB_STM32L496VGT6P_RxCpltCallback( uint8_t * pbuf, uint32_t * Len );
void USB_STM32L496VGT6P_TxCpltCallback( uint8_t * pbuf, uint32_t * Len );

void OTG_FS_IRQHandler( void );

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Write( USB_STM32L496VGT6P_Instance_t * Instance, USB_STM32L496VGT6P_Data_t * Data, USB_STM32L496VGT6P_DataLength_t DataLength );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Read( USB_STM32L496VGT6P_Instance_t * Instance, USB_STM32L496VGT6P_Data_t * Data, USB_STM32L496VGT6P_DataLength_t DataLength );

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Initialize( USB_STM32L496VGT6P_Instance_t * Instance );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Cycle( USB_STM32L496VGT6P_Instance_t * Instance );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_DeInitialize( USB_STM32L496VGT6P_Instance_t * Instance );
//
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Context_Initialize( void );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Context_Cycle( void );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Context_DeInitialize( void );

// #############################################################################
// #### Private Variable(s) ####################################################
// #############################################################################

static USB_STM32L496VGT6P_Context_t USB_STM32L496VGT6P_Context;

// #############################################################################
// #### Private Method(s) ######################################################
// #############################################################################

static GPIO_Status_t GPIO_CallbackOnInterrupt( GPIO_t GPIOx, GPIO_ContextOnInterrupt_t Context )
{
    GPIO_Status_t GPIO_Status = GPIO_Status_Success;

    do
    {
        USB_Debug( "%s%s( GPIOx=%d, Context=%p )", UTIL_StringConcatenateConstant( UTIL_CSI_SelectGraphicRendition( UTIL_CSI_SelectGraphicRenditionColorForegroundBlack ), UTIL_CSI_SelectGraphicRendition( UTIL_CSI_SelectGraphicRenditionColorBackgroundCyan ) ), __FUNCTION__, GPIOx, Context );

        USB_STM32L496VGT6P_Instance_t * Instance = Context;
        if ( GPIOx == Instance->VBUS_Sense )
        {
            PCD_HandleTypeDef * hpcd = &Instance->Context->USBx;
            HAL_PCDEx_BCD_VBUSDetect( hpcd );
        }
    }
    while ( 0 );

    return GPIO_Status;
}

void USB_STM32L496VGT6P_RxCpltCallback( uint8_t * pbuf, uint32_t * Len )
{
    #if 1
        // TODO
    #else
    USB_STM32L496VGT6P_Instance_t * Instance = USB_Context.Instance[ USB_1 ].STM32L496VGT6P;
    if ( Instance->Context->Receive.Length >= UTIL_SizeOf( Instance->Context->Receive.Content ) )
    {
        USB_Warning( "%s: %s", __FUNCTION__, "MAX Length Reached!" );
        Instance->Context->Receive.Length = 0;
    }
    UTIL_MemoryCopy( &Instance->Context->Receive.Content[ Instance->Context->Receive.Length ], pbuf, *Len );
    Instance->Context->Receive.Length += *Len;
    #endif
}

void USB_STM32L496VGT6P_TxCpltCallback( uint8_t * pbuf, uint32_t * Len )
{
    #if 1
        // TODO
    #else
    USB_STM32L496VGT6P_Instance_t * Instance = USB_Context.Instance[ USB_1 ].STM32L496VGT6P;
    if ( *Len <= Instance->Context->Transmit.Length )
    {
        UTIL_MemoryCopy( &Instance->Context->Transmit.Content[ 0 ], &Instance->Context->Transmit.Content[ *Len ], Instance->Context->Transmit.Length - *Len );
        Instance->Context->Transmit.Length -= *Len;
        Instance->Context->Status = USB_STM32L496VGT6P_Status_Success;
    }
    else
    {
        USB_Error( "%s: Unexpected Size %d out of %d", __FUNCTION__, *Len, Instance->Context->Transmit.Length );

        Instance->Context->Transmit.Length = 0;
        Instance->Context->Status = USB_STM32L496VGT6P_Status_Error;
    }
    #endif
}

void OTG_FS_IRQHandler( void )
{
    PCD_HandleTypeDef * hpcd = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ].USBx;
    HAL_PCD_IRQHandler( hpcd );
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Write( USB_STM32L496VGT6P_Instance_t * Instance, USB_STM32L496VGT6P_Data_t * Data, USB_STM32L496VGT6P_DataLength_t DataLength )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Error;
    do
    {
        USB_Trace( "%s( Instance=%p, Data=%p, Length=%d )", __FUNCTION__, Instance, Data, DataLength );

        if ( Instance == NULL )
        {
            Status = USB_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        if ( Instance->Context->Status == USB_STM32L496VGT6P_Status_Busy )
        {
            Status = USB_STM32L496VGT6P_Status_Busy;
            break;
        }
        if ( Instance->Context->Transmit.Length + DataLength >= UTIL_SizeOf( Instance->Context->Transmit.Content ) )
        {
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }
        UTIL_MemoryCopy( &Instance->Context->Transmit.Content[ Instance->Context->Transmit.Length ], Data, DataLength );
        Instance->Context->Transmit.Length += DataLength;

        Status = USB_STM32L496VGT6P_Status_Success;
    }
    while ( 0 );
    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Read( USB_STM32L496VGT6P_Instance_t * Instance, USB_STM32L496VGT6P_Data_t * Data, USB_STM32L496VGT6P_DataLength_t DataLength )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Error;
    do
    {
        USB_Trace( "%s( Instance=%p, Data=%p, Length=%d )", __FUNCTION__, Instance, Data, DataLength );

        if ( Instance == NULL )
        {
            Status = USB_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        if ( Instance->Context->Receive.Length < DataLength )
        {
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }
        UTIL_MemoryCopy( Data, &Instance->Context->Receive.Content[ 0 ], DataLength );
        KERNEL_InterruptDisable( );
        UTIL_MemoryCopy( &Instance->Context->Receive.Content[ 0 ], &Instance->Context->Receive.Content[ DataLength ], Instance->Context->Receive.Length - DataLength );
        Instance->Context->Receive.Length -= DataLength;
        KERNEL_InterruptEnable( );
        Status = USB_STM32L496VGT6P_Status_Success;
    }
    while ( 0 );
    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Initialize( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Error;
    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( Instance == NULL )
        {
            Status = USB_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ Instance->USBx ];

        Status = USB_STM32L496VGT6P_Status_Success;

        switch ( Instance->USBx )
        {
            case USB_STM32L496VGT6P_1:
                GPIO_Status_t GPIO_Status = GPIO_Status_Success;

                // TODO Configure GPIOs

                if ( ( GPIO_Status = GPIO_SetCallbackOnInterrupt( Instance->VBUS_Sense, GPIO_CallbackOnInterrupt, Instance ) ) != GPIO_Status_Success )
                {
                    Status = USB_Status_Error;
                    break;
                }
                break;

            default:
                Status = USB_STM32L496VGT6P_Status_NotSupported;
                break;
        }
        if ( Status != USB_STM32L496VGT6P_Status_Success )
        {
            break;
        }

        Instance->Context = Context;
    }
    while ( 0 );
    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Cycle( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Error;
    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( Instance == NULL )
        {
            Status = USB_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        Status = USB_STM32L496VGT6P_Status_Success;
        switch ( Instance->Context->Status )
        {
            case USB_STM32L496VGT6P_Status_Busy:
                // Wait for end of action
                break;
            default:
                if ( Instance->Context->Transmit.Length > 0 )
                {
                    // FIXME
                    //          if ( CDC_Transmit_FS( Instance->Context->Transmit.Content, Instance->Context->Transmit.Length ) != USBD_OK )
                    //          {
                    //            Status = USB_STM32L496VGT6P_Status_Error;
                    //            break;
                    //          }
                    Instance->Context->Status = USB_STM32L496VGT6P_Status_Busy;
                }
                break;
        }
    }
    while ( 0 );
    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_DeInitialize( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Error;
    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( Instance == NULL )
        {
            Status = USB_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        // FIXME
        Status = USB_STM32L496VGT6P_Status_NotSupported;
    }
    while ( 0 );
    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Context_Initialize( void )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Error;
    do
    {
        USB_Trace( "%s( void )", __FUNCTION__ );

        // FIXME Remove the usage of `MX_USB_DEVICE_Init()`
    #if 1
        extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
        extern void MX_USB_DEVICE_Init( void );
        MX_USB_DEVICE_Init( );
        USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ].USBx = hpcd_USB_OTG_FS;
        // FIX Let the USBD use our instance
        ( ( USBD_HandleTypeDef * ) ( hpcd_USB_OTG_FS.pData ) )->pData = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ].USBx;
    #endif

        Status = USB_STM32L496VGT6P_Status_Success;
    }
    while ( 0 );
    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Context_Cycle( void )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Error;
    do
    {
        USB_Trace( "%s( void )", __FUNCTION__ );
        UTIL_UNUSED( USB_STM32L496VGT6P_Context ); // FIXME Skip Warning
        // Nothing to be done
        Status = USB_STM32L496VGT6P_Status_Success;
    }
    while ( 0 );
    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Context_DeInitialize( void )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Error;
    do
    {
        USB_Trace( "%s( void )", __FUNCTION__ );
        UTIL_UNUSED( USB_STM32L496VGT6P_Context ); // FIXME Skip Warning
        // Nothing to be done
        Status = USB_STM32L496VGT6P_Status_Success;
    }
    while ( 0 );
    return Status;
}

// #############################################################################
// #### Public Method(s) #######################################################
// #############################################################################

USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Initialize( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Error;
    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );
        if ( ( Status = USB_STM32L496VGT6P_Context_Initialize( ) ) != USB_STM32L496VGT6P_Status_Success )
        {
            break;
        }
        Status = USB_STM32L496VGT6P_Instance_Initialize( Instance );
    }
    while ( 0 );
    return Status;
}

USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Cycle( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Error;
    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );
        if ( ( Status = USB_STM32L496VGT6P_Context_Cycle( ) ) != USB_STM32L496VGT6P_Status_Success )
        {
            break;
        }
        Status = USB_STM32L496VGT6P_Instance_Cycle( Instance );
    }
    while ( 0 );
    return Status;
}

USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_DeInitialize( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Error;
    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );
        if ( ( Status = USB_STM32L496VGT6P_Instance_DeInitialize( Instance ) ) != USB_STM32L496VGT6P_Status_Success )
        {
            break;
        }
        Status = USB_STM32L496VGT6P_Context_DeInitialize( );
    }
    while ( 0 );
    return Status;
}

USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Write( USB_STM32L496VGT6P_Instance_t * Instance, USB_STM32L496VGT6P_Data_t * Data, USB_STM32L496VGT6P_DataLength_t DataLength )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Error;
    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( Instance == NULL )
        {
            Status = USB_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        Status = USB_STM32L496VGT6P_Instance_Write( Instance, Data, DataLength );
    }
    while ( 0 );
    return Status;
}

USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Read( USB_STM32L496VGT6P_Instance_t * Instance, USB_STM32L496VGT6P_Data_t * Data, USB_STM32L496VGT6P_DataLength_t DataLength )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Error;
    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( Instance == NULL )
        {
            Status = USB_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        Status = USB_STM32L496VGT6P_Instance_Read( Instance, Data, DataLength );
    }
    while ( 0 );
    return Status;
}

// #############################################################################
// #### Public Variable(s) #####################################################
// #############################################################################

const char USB_STM32L496VGT6P_VERSION[] = "0.0.0.v20260117-1502";

// #############################################################################
// #### File Guard #############################################################
// #############################################################################

#endif /* STM32L496xx */

// #############################################################################
// #### END OF FILE ############################################################
// #############################################################################
