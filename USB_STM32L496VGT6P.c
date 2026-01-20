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
    #include "usbd_core.h"
    #include "usbd_def.h"

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
    USB_STM32L496VGT6P_Event_Interrupt = UTIL_BIT( 0 ),
    USB_STM32L496VGT6P_Event_Connected = UTIL_BIT( 1 ),
    USB_STM32L496VGT6P_Event_Disconnected = UTIL_BIT( 2 ),
    USB_STM32L496VGT6P_Event_StartOfFrame = UTIL_BIT( 3 ),
    USB_STM32L496VGT6P_Event_Setup = UTIL_BIT( 4 ),
    USB_STM32L496VGT6P_Event_Reset = UTIL_BIT( 5 ),
    USB_STM32L496VGT6P_Event_Suspend = UTIL_BIT( 6 ),
    USB_STM32L496VGT6P_Event_Resume = UTIL_BIT( 7 ),
    USB_STM32L496VGT6P_Event_Tx = UTIL_BIT( 8 ),
    USB_STM32L496VGT6P_Event_Rx = UTIL_BIT( 9 ),
    USB_STM32L496VGT6P_Event_TxIncomplete = UTIL_BIT( 10 ),
    USB_STM32L496VGT6P_Event_RxIncomplete = UTIL_BIT( 11 ),
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
    // TODO Transform the following to Process->Operation paradigm
    volatile USB_STM32L496VGT6P_Status_t Status;
    USB_STM32L496VGT6P_Buffer_Receive_t Receive;
    USB_STM32L496VGT6P_Buffer_Transmit_t Transmit;
    USB_STM32L496VGT6P_Event_t Event;
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

void HAL_PCD_SOFCallback( PCD_HandleTypeDef * hpcd );
void HAL_PCD_SetupStageCallback( PCD_HandleTypeDef * hpcd );
void HAL_PCD_ResetCallback( PCD_HandleTypeDef * hpcd );
void HAL_PCD_SuspendCallback( PCD_HandleTypeDef * hpcd );
void HAL_PCD_ResumeCallback( PCD_HandleTypeDef * hpcd );
void HAL_PCD_ConnectCallback( PCD_HandleTypeDef * hpcd );
void HAL_PCD_DisconnectCallback( PCD_HandleTypeDef * hpcd );

void HAL_PCD_DataOutStageCallback( PCD_HandleTypeDef * hpcd, uint8_t epnum );
void HAL_PCD_DataInStageCallback( PCD_HandleTypeDef * hpcd, uint8_t epnum );
void HAL_PCD_ISOOUTIncompleteCallback( PCD_HandleTypeDef * hpcd, uint8_t epnum );
void HAL_PCD_ISOINIncompleteCallback( PCD_HandleTypeDef * hpcd, uint8_t epnum );

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
    USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ];

    Context->Event |= USB_STM32L496VGT6P_Event_Interrupt;

    PCD_HandleTypeDef * hpcd = &Context->USBx;
    HAL_PCD_IRQHandler( hpcd );
}

void HAL_PCD_SOFCallback( PCD_HandleTypeDef * hpcd )
{
    USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ];

    Context->Event |= USB_STM32L496VGT6P_Event_StartOfFrame;

    USBD_LL_SOF( ( USBD_HandleTypeDef * ) hpcd->pData );
}

void HAL_PCD_SetupStageCallback( PCD_HandleTypeDef * hpcd )
{
    USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ];

    Context->Event |= USB_STM32L496VGT6P_Event_Setup;

    USBD_LL_SetupStage( ( USBD_HandleTypeDef * ) hpcd->pData, ( uint8_t * ) hpcd->Setup );
}

void HAL_PCD_ResetCallback( PCD_HandleTypeDef * hpcd )
{
    USBD_SpeedTypeDef speed = USBD_SPEED_FULL;
    USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ];

    Context->Event |= USB_STM32L496VGT6P_Event_Reset;

    if ( hpcd->Init.speed != PCD_SPEED_FULL )
    {
        Error_Handler( );
    }
    /* Set Speed. */
    USBD_LL_SetSpeed( ( USBD_HandleTypeDef * ) hpcd->pData, speed );

    /* Reset Device. */
    USBD_LL_Reset( ( USBD_HandleTypeDef * ) hpcd->pData );
}

void HAL_PCD_SuspendCallback( PCD_HandleTypeDef * hpcd )
{
    USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ];

    Context->Event |= USB_STM32L496VGT6P_Event_Suspend;

    __HAL_PCD_GATE_PHYCLOCK( hpcd );
    /* Inform USB library that core enters in suspend Mode. */
    USBD_LL_Suspend( ( USBD_HandleTypeDef * ) hpcd->pData );
    /* Enter in STOP mode. */
    /* USER CODE BEGIN 2 */
    if ( hpcd->Init.low_power_enable )
    {
        /* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
        SCB->SCR |= ( uint32_t ) ( ( uint32_t ) ( SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk ) );
    }
    /* USER CODE END 2 */
}

void HAL_PCD_ResumeCallback( PCD_HandleTypeDef * hpcd )
{
    USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ];

    Context->Event |= USB_STM32L496VGT6P_Event_Resume;

    __HAL_PCD_UNGATE_PHYCLOCK( hpcd );

    /* USER CODE BEGIN 3 */
    if ( hpcd->Init.low_power_enable )
    {
        /* Reset SLEEPDEEP bit of Cortex System Control Register. */
        SCB->SCR &= ( uint32_t ) ~( ( uint32_t ) ( SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk ) );
        // TODO Let the following being done through the kernel or suitable module
        // FIXME Instead of using SystemClockConfig_Resume();
        //       Use directly SystemClock_Config();
        extern void SystemClock_Config( void );
        SystemClock_Config( );
    }
    /* USER CODE END 3 */
    USBD_LL_Resume( ( USBD_HandleTypeDef * ) hpcd->pData );
}

void HAL_PCD_ConnectCallback( PCD_HandleTypeDef * hpcd )
{
    USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ];

    Context->Event |= USB_STM32L496VGT6P_Event_Connected;

    USBD_LL_DevConnected( ( USBD_HandleTypeDef * ) hpcd->pData );
}

void HAL_PCD_DisconnectCallback( PCD_HandleTypeDef * hpcd )
{
    USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ];

    Context->Event |= USB_STM32L496VGT6P_Event_Disconnected;

    USBD_LL_DevDisconnected( ( USBD_HandleTypeDef * ) hpcd->pData );
}

void HAL_PCD_DataOutStageCallback( PCD_HandleTypeDef * hpcd, uint8_t epnum )
{
    USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ];

    Context->Event |= USB_STM32L496VGT6P_Event_Rx;

    USBD_LL_DataOutStage( ( USBD_HandleTypeDef * ) hpcd->pData, epnum, hpcd->OUT_ep[ epnum ].xfer_buff );
}

void HAL_PCD_DataInStageCallback( PCD_HandleTypeDef * hpcd, uint8_t epnum )
{
    USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ];

    Context->Event |= USB_STM32L496VGT6P_Event_Tx;

    USBD_LL_DataInStage( ( USBD_HandleTypeDef * ) hpcd->pData, epnum, hpcd->IN_ep[ epnum ].xfer_buff );
}

void HAL_PCD_ISOOUTIncompleteCallback( PCD_HandleTypeDef * hpcd, uint8_t epnum )
{
    USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ];

    Context->Event |= USB_STM32L496VGT6P_Event_RxIncomplete;

    USBD_LL_IsoOUTIncomplete( ( USBD_HandleTypeDef * ) hpcd->pData, epnum );
}

void HAL_PCD_ISOINIncompleteCallback( PCD_HandleTypeDef * hpcd, uint8_t epnum )
{
    USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ];

    Context->Event |= USB_STM32L496VGT6P_Event_TxIncomplete;

    USBD_LL_IsoINIncomplete( ( USBD_HandleTypeDef * ) hpcd->pData, epnum );
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

        if ( ( Instance->Context->Event & USB_STM32L496VGT6P_Event_Interrupt ) != 0 )
        {
            Instance->Context->Event &= ~USB_STM32L496VGT6P_Event_Interrupt;
            USB_Trace( "Interrupt: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & USB_STM32L496VGT6P_Event_Connected ) != 0 )
        {
            Instance->Context->Event &= ~USB_STM32L496VGT6P_Event_Connected;
            USB_Debug( "Connected: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & USB_STM32L496VGT6P_Event_Disconnected ) != 0 )
        {
            Instance->Context->Event &= ~USB_STM32L496VGT6P_Event_Disconnected;
            USB_Debug( "DisConnected: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & USB_STM32L496VGT6P_Event_StartOfFrame ) != 0 )
        {
            Instance->Context->Event &= ~USB_STM32L496VGT6P_Event_StartOfFrame;
            USB_Debug( "Start Of Frame: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & USB_STM32L496VGT6P_Event_Setup ) != 0 )
        {
            Instance->Context->Event &= ~USB_STM32L496VGT6P_Event_Setup;
            USB_Debug( "Setup: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & USB_STM32L496VGT6P_Event_Reset ) != 0 )
        {
            Instance->Context->Event &= ~USB_STM32L496VGT6P_Event_Reset;
            USB_Debug( "Reset: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & USB_STM32L496VGT6P_Event_Suspend ) != 0 )
        {
            Instance->Context->Event &= ~USB_STM32L496VGT6P_Event_Suspend;
            USB_Debug( "Suspend: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & USB_STM32L496VGT6P_Event_Resume ) != 0 )
        {
            Instance->Context->Event &= ~USB_STM32L496VGT6P_Event_Resume;
            USB_Debug( "Resume: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & USB_STM32L496VGT6P_Event_Tx ) != 0 )
        {
            Instance->Context->Event &= ~USB_STM32L496VGT6P_Event_Tx;
            USB_Debug( "TX Complete: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & USB_STM32L496VGT6P_Event_Rx ) != 0 )
        {
            Instance->Context->Event &= ~USB_STM32L496VGT6P_Event_Rx;
            USB_Debug( "RX Complete: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & USB_STM32L496VGT6P_Event_TxIncomplete ) != 0 )
        {
            Instance->Context->Event &= ~USB_STM32L496VGT6P_Event_TxIncomplete;
            USB_Debug( "TX Incomplete: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Instance->Context->Event & USB_STM32L496VGT6P_Event_RxIncomplete ) != 0 )
        {
            Instance->Context->Event &= ~USB_STM32L496VGT6P_Event_RxIncomplete;
            USB_Debug( "RX Incomplete: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
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

const char USB_STM32L496VGT6P_VERSION[] = "0.0.0.v20260120-0211";

// #############################################################################
// #### File Guard #############################################################
// #############################################################################

#endif /* STM32L496xx */

// #############################################################################
// #### END OF FILE ############################################################
// #############################################################################
