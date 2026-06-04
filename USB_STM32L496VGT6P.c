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
// #undef DEBUG // FIXME Un-Comment to disable logging within this file
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
    #include "usbd_cdc_if.h"
    #include "usbd_core.h"
    #include "usbd_def.h"

// #############################################################################
// #### Private Macro(s) #######################################################
// #############################################################################

// #############################################################################
// #### Private Type(s) ########################################################
// #############################################################################

/**
 * @brief USB STM32L496VGT6P Operation
 *
 * @enum USB_STM32L496VGT6P_OperationType_t
 */
typedef enum USB_STM32L496VGT6P_OperationType
{
    USB_STM32L496VGT6P_OperationType_None = 0, ///< None
    USB_STM32L496VGT6P_OperationType_Pending,  ///< Pending
    USB_STM32L496VGT6P_OperationType_Commit,   ///< Commit
} USB_STM32L496VGT6P_OperationType_t;

/**
 * @brief USB STM32L496VGT6P Operation Handler
 */
typedef USB_STM32L496VGT6P_Status_t ( *USB_STM32L496VGT6P_OperationHandler_t )( USB_STM32L496VGT6P_t USBx );

/**
 * @brief USB STM32L496VGT6P Operation Context
 *
 * @struct USB_STM32L496VGT6P_OperationContext_t
 */
typedef struct USB_STM32L496VGT6P_OperationContext
{
} USB_STM32L496VGT6P_OperationContext_t;

/**
 * @brief USB STM32L496VGT6P Operation
 *
 * @struct USB_STM32L496VGT6P_Operation_t
 */
typedef struct USB_STM32L496VGT6P_Operation
{
    USB_STM32L496VGT6P_OperationType_t Type;       ///< Type
    USB_STM32L496VGT6P_OperationHandler_t Handler; ///< Handler
    USB_STM32L496VGT6P_Status_t Status;            ///< Status
    TIM_Timestamp_t Timeout;                       ///< Timeout
    USB_STM32L496VGT6P_OperationContext_t Context; ///< Context
} USB_STM32L496VGT6P_Operation_t;

/**
 * @brief USB STM32L496VGT6P Process Type
 *
 * @enum USB_STM32L496VGT6P_ProcessType_t
 */
typedef enum USB_STM32L496VGT6P_ProcessType
{
    USB_STM32L496VGT6P_ProcessType_None = 0,   ///< None
    USB_STM32L496VGT6P_ProcessType_Initialize, ///< Initialize
} USB_STM32L496VGT6P_ProcessType_t;

/**
 * @brief USB STM32L496VGT6P Process Handler
 */
typedef USB_STM32L496VGT6P_Status_t ( *USB_STM32L496VGT6P_ProcessHandler_t )( USB_STM32L496VGT6P_t USBx );

/**
 * @brief USB STM32L496VGT6P Process Context
 *
 * @struct USB_STM32L496VGT6P_ProcessContext_t
 */
typedef struct USB_STM32L496VGT6P_ProcessContext
{
    USB_STM32L496VGT6P_Operation_t Operation; ///< Operation
} USB_STM32L496VGT6P_ProcessContext_t;

/**
 * @brief USB STM32L496VGT6P Process
 *
 * @struct USB_STM32L496VGT6P_Process_t
 */
typedef struct USB_STM32L496VGT6P_Process
{
    USB_STM32L496VGT6P_ProcessType_t Type;       ///< Type
    USB_STM32L496VGT6P_ProcessHandler_t Handler; ///< Handler
    USB_STM32L496VGT6P_ProcessContext_t Context; ///< Context
} USB_STM32L496VGT6P_Process_t;

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
    USB_STM32L496VGT6P_Event_TxComplete = UTIL_BIT( 10 ),
    USB_STM32L496VGT6P_Event_RxComplete = UTIL_BIT( 11 ),
    USB_STM32L496VGT6P_Event_TxIncomplete = UTIL_BIT( 12 ),
    USB_STM32L496VGT6P_Event_RxIncomplete = UTIL_BIT( 13 ),
    USB_STM32L496VGT6P_Event_DataTerminalReady_Enabled = UTIL_BIT( 14 ),
    USB_STM32L496VGT6P_Event_DataTerminalReady_Disabled = UTIL_BIT( 15 ),
} USB_STM32L496VGT6P_Event_t;

typedef enum USB_STM32L496VGT6P_DataTerminalReady
{
    USB_STM32L496VGT6P_DataTerminalReady_Disabled = 0,
    USB_STM32L496VGT6P_DataTerminalReady_Enabled,
} USB_STM32L496VGT6P_DataTerminalReady_t;

typedef struct USB_STM32L496VGT6P_InterfaceContext
{
    USB_STM32L496VGT6P_DataTerminalReady_t DataTerminalReady;
    BUFFER_t Transmit;
    BUFFER_t Receive;
} USB_STM32L496VGT6P_InterfaceContext_t;

typedef struct USB_STM32L496VGT6P_Instance
{
    GPIO_t VBUS_Sense;

    PCD_HandleTypeDef USBx;

    USB_STM32L496VGT6P_InterfaceContext_t Interface[ USB_STM32L496VGT6P_Interface_Count ];

    USB_STM32L496VGT6P_Event_t Event;

    USB_STM32L496VGT6P_Process_t Process;
} USB_STM32L496VGT6P_Instance_t;

typedef struct USB_STM32L496VGT6P_Context
{
    TIM_Timestamp_t Timestamp;
    USB_STM32L496VGT6P_Instance_t Instance[ USB_STM32L496VGT6P_Count ];
} USB_STM32L496VGT6P_Context_t;

// #############################################################################
// #### Private Method(s) Prototype ############################################
// #############################################################################

static GPIO_Status_t GPIO_CallbackOnInterrupt( GPIO_t GPIOx, GPIO_CallbackContext_t * Context );

void USB_STM32L496VGT6P_RxCpltCallback( uint8_t * pbuf, uint32_t * Len, uint8_t Interface );
void USB_STM32L496VGT6P_TxCpltCallback( uint8_t * pbuf, uint32_t * Len, uint8_t Interface );

void USB_STM32L496VGT6P_DataTerminalReadyCallback( uint8_t DTR, uint8_t Interface );

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

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Context_Initialize( void );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Context_Cycle( void );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Context_DeInitialize( void );

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Initialize( USB_STM32L496VGT6P_t USBx );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Cycle( USB_STM32L496VGT6P_t USBx );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_DeInitialize( USB_STM32L496VGT6P_t USBx );

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_SetProcess( USB_STM32L496VGT6P_t USBx, USB_STM32L496VGT6P_ProcessType_t ProcessType );

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_ProcessInitialize( USB_STM32L496VGT6P_t USBx );

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationCommitExecute( USB_STM32L496VGT6P_t USBx );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationCommitResolve( USB_STM32L496VGT6P_t USBx );

// #############################################################################
// #### Private Variable(s) ####################################################
// #############################################################################

static USB_STM32L496VGT6P_Context_t USB_STM32L496VGT6P_Context;

// #############################################################################
// #### Private Method(s) ######################################################
// #############################################################################

static GPIO_Status_t GPIO_CallbackOnInterrupt( GPIO_t GPIOx, GPIO_CallbackContext_t * GPIO_Context )
{
    GPIO_Status_t GPIO_Status = GPIO_Status_Success;

    do
    {
        USB_Debug( "%s%s( GPIOx=%d, Context=%p )", UTIL_StringConcatenateConstant( UTIL_CSI_SelectGraphicRendition( UTIL_CSI_SelectGraphicRenditionColorForegroundBlack ), UTIL_CSI_SelectGraphicRendition( UTIL_CSI_SelectGraphicRenditionColorBackgroundCyan ) ), __FUNCTION__, GPIOx, GPIO_Context );

        USB_STM32L496VGT6P_Instance_t * Instance = ( USB_STM32L496VGT6P_Instance_t * ) GPIO_Context;

        /*
         * @brief  Handle USB VBUS detection upon external interrupt
         */
        PCD_HandleTypeDef * hpcd = &Instance->USBx;
        HAL_PCDEx_BCD_VBUSDetect( hpcd );
    }
    while ( 0 );

    return GPIO_Status;
}

void USB_STM32L496VGT6P_RxCpltCallback( uint8_t * pbuf, uint32_t * Len, uint8_t Interface )
{
    BUFFER_Status_t BUFFER_Status = BUFFER_Status_Success;

    USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];

    Instance->Event |= USB_STM32L496VGT6P_Event_RxComplete;

    if ( ( BUFFER_Status = BUFFER_PushMemoryBackCircular( &Instance->Interface[ Interface ].Receive, pbuf, *Len ) ) != BUFFER_Status_Success )
    {
        USB_Warning( "%s: %s", __FUNCTION__, "Receive Buffer FULL!" );
    }
}

void USB_STM32L496VGT6P_TxCpltCallback( uint8_t * pbuf, uint32_t * Len, uint8_t Interface )
{
    BUFFER_Status_t BUFFER_Status = BUFFER_Status_Success;

    USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];

    Instance->Event |= USB_STM32L496VGT6P_Event_TxComplete;

    if ( ( BUFFER_Status = BUFFER_Reset( &Instance->Interface[ Interface ].Transmit ) ) != BUFFER_Status_Success )
    {
        // FIXME
    }

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

void USB_STM32L496VGT6P_DataTerminalReadyCallback( uint8_t DTR, uint8_t Interface )
{
    USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];

    if ( DTR )
    {
        Instance->Event |= USB_STM32L496VGT6P_Event_DataTerminalReady_Enabled;
        Instance->Interface[ Interface ].DataTerminalReady = USB_STM32L496VGT6P_DataTerminalReady_Enabled;
    }
    else
    {
        Instance->Event |= USB_STM32L496VGT6P_Event_DataTerminalReady_Disabled;
        Instance->Interface[ Interface ].DataTerminalReady = USB_STM32L496VGT6P_DataTerminalReady_Disabled;
    }
}

void OTG_FS_IRQHandler( void )
{
    USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];

    Instance->Event |= USB_STM32L496VGT6P_Event_Interrupt;

    PCD_HandleTypeDef * hpcd = &Instance->USBx;
    HAL_PCD_IRQHandler( hpcd );
}

void HAL_PCD_SOFCallback( PCD_HandleTypeDef * hpcd )
{
    USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];

    Instance->Event |= USB_STM32L496VGT6P_Event_StartOfFrame;

    USBD_LL_SOF( ( USBD_HandleTypeDef * ) hpcd->pData );
}

void HAL_PCD_SetupStageCallback( PCD_HandleTypeDef * hpcd )
{
    USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];

    Instance->Event |= USB_STM32L496VGT6P_Event_Setup;

    USBD_LL_SetupStage( ( USBD_HandleTypeDef * ) hpcd->pData, ( uint8_t * ) hpcd->Setup );
}

void HAL_PCD_ResetCallback( PCD_HandleTypeDef * hpcd )
{
    USBD_SpeedTypeDef speed = USBD_SPEED_FULL;
    USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];

    Instance->Event |= USB_STM32L496VGT6P_Event_Reset;

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
    USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];

    Instance->Event |= USB_STM32L496VGT6P_Event_Suspend;

    __HAL_PCD_GATE_PHYCLOCK( hpcd );
    /* Inform USB library that core enters in suspend Mode. */
    USBD_LL_Suspend( ( USBD_HandleTypeDef * ) hpcd->pData );
    /* Enter in STOP mode. */
    if ( hpcd->Init.low_power_enable )
    {
        /* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
        SCB->SCR |= ( uint32_t ) ( ( uint32_t ) ( SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk ) );
    }
}

void HAL_PCD_ResumeCallback( PCD_HandleTypeDef * hpcd )
{
    USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];

    Instance->Event |= USB_STM32L496VGT6P_Event_Resume;

    __HAL_PCD_UNGATE_PHYCLOCK( hpcd );

    if ( hpcd->Init.low_power_enable )
    {
        /* Reset SLEEPDEEP bit of Cortex System Control Register. */
        SCB->SCR &= ( uint32_t ) ~( ( uint32_t ) ( SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk ) );
        // FIXME Instead of using SystemClockConfig_Resume();
        //       Use directly SystemClock_Config() through KERNEL;
        KERNEL_ClockEnable( KERNEL_All );
    }
    USBD_LL_Resume( ( USBD_HandleTypeDef * ) hpcd->pData );
}

void HAL_PCD_ConnectCallback( PCD_HandleTypeDef * hpcd )
{
    USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];

    Instance->Event |= USB_STM32L496VGT6P_Event_Connected;

    USBD_LL_DevConnected( ( USBD_HandleTypeDef * ) hpcd->pData );
}

void HAL_PCD_DisconnectCallback( PCD_HandleTypeDef * hpcd )
{
    USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];

    Instance->Event |= USB_STM32L496VGT6P_Event_Disconnected;

    USBD_LL_DevDisconnected( ( USBD_HandleTypeDef * ) hpcd->pData );
}

void HAL_PCD_DataOutStageCallback( PCD_HandleTypeDef * hpcd, uint8_t epnum )
{
    USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];

    Instance->Event |= USB_STM32L496VGT6P_Event_Rx;

    USBD_LL_DataOutStage( ( USBD_HandleTypeDef * ) hpcd->pData, epnum, hpcd->OUT_ep[ epnum ].xfer_buff );
}

void HAL_PCD_DataInStageCallback( PCD_HandleTypeDef * hpcd, uint8_t epnum )
{
    USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];

    Instance->Event |= USB_STM32L496VGT6P_Event_Tx;

    USBD_LL_DataInStage( ( USBD_HandleTypeDef * ) hpcd->pData, epnum, hpcd->IN_ep[ epnum ].xfer_buff );
}

void HAL_PCD_ISOOUTIncompleteCallback( PCD_HandleTypeDef * hpcd, uint8_t epnum )
{
    USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];

    Instance->Event |= USB_STM32L496VGT6P_Event_RxIncomplete;

    USBD_LL_IsoOUTIncomplete( ( USBD_HandleTypeDef * ) hpcd->pData, epnum );
}

void HAL_PCD_ISOINIncompleteCallback( PCD_HandleTypeDef * hpcd, uint8_t epnum )
{
    USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];

    Instance->Event |= USB_STM32L496VGT6P_Event_TxIncomplete;

    USBD_LL_IsoINIncomplete( ( USBD_HandleTypeDef * ) hpcd->pData, epnum );
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Context_Initialize( void )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;
    BUFFER_Status_t BUFFER_Status = BUFFER_Status_Success;

    do
    {
        USB_Trace( "%s( void )", __FUNCTION__ );

        TIM_Status_t TIM_Status = TIM_Status_Error;
        if ( ( TIM_Status = TIM_GetTimestamp( USB_TIM, &USB_STM32L496VGT6P_Context.Timestamp ) ) != TIM_Status_Success )
        {
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }

        // FIXME Remove the usage of `MX_USB_DEVICE_Init()`
    #if 1
        extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
        extern void MX_USB_DEVICE_Init( void );
        MX_USB_DEVICE_Init( );
        USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ];
        Instance->USBx = hpcd_USB_OTG_FS;
        // FIX Let the USBD use our instance
        // ( ( USBD_HandleTypeDef * ) ( hpcd_USB_OTG_FS.pData ) )->pData = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ].USBx; // @note Replaced by the following
        USBD_HandleTypeDef * pdev = ( USBD_HandleTypeDef * ) hpcd_USB_OTG_FS.pData;
        pdev->pData = &USB_STM32L496VGT6P_Context.Instance[ USB_STM32L496VGT6P_1 ].USBx;

        // FIXME the following is skipped due to USB enumeration still in progress on this stage
        // @note the following follows @ref MX_USB_DEVICE_Init() interface registration
        for ( USB_STM32L496VGT6P_Interface_t Interface = USB_STM32L496VGT6P_Interface_1; Interface < USB_STM32L496VGT6P_Interface_Count; ++Interface )
        {
            uint32_t classId = USBD_CMPSIT_GetClassID( pdev, CLASS_TYPE_CDC, Interface );
            if ( classId == 0xFFU )
            {
                USB_Warning( "Couldn't Retrieve CDC Interface %d Class ID", Interface );
                continue;
            }

            USB_Debug( "CDC Interface %d Has Class ID %d", Interface, classId );

            USB_Warning( "CDC Interface Buffers initialization has been skipped" );
            continue;

            USBD_CDC_HandleTypeDef * USB_Device_CDC = ( USBD_CDC_HandleTypeDef * ) pdev->pClassDataCmsit[ classId ];
            if ( ( BUFFER_Status = BUFFER_Initialize( &Instance->Interface[ Interface ].Transmit, USB_Device_CDC->TxBuffer, USB_Device_CDC->TxLength ) ) != BUFFER_Status_Success )
            {
                Status = USB_STM32L496VGT6P_Status_Error;
                break;
            }

            if ( ( BUFFER_Status = BUFFER_Initialize( &Instance->Interface[ Interface ].Receive, USB_Device_CDC->RxBuffer, USB_Device_CDC->RxLength ) ) != BUFFER_Status_Success )
            {
                Status = USB_STM32L496VGT6P_Status_Error;
                break;
            }
        }
    #endif
    }
    while ( 0 );

    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Context_Cycle( void )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( void )", __FUNCTION__ );

        TIM_Status_t TIM_Status = TIM_Status_Error;
        if ( ( TIM_Status = TIM_GetTimestamp( USB_TIM, &USB_STM32L496VGT6P_Context.Timestamp ) ) != TIM_Status_Success )
        {
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }
    }
    while ( 0 );

    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Context_DeInitialize( void )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( void )", __FUNCTION__ );

        UTIL_UNUSED( USB_STM32L496VGT6P_Context );
    }
    while ( 0 );

    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Initialize( USB_STM32L496VGT6P_t USBx )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;
    GPIO_Status_t GPIO_Status = GPIO_Status_Success;

    do
    {
        USB_Trace( "%s( USBx=%d )", __FUNCTION__, USBx );

        USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USBx ];

        switch ( USBx )
        {
            case USB_STM32L496VGT6P_1:
                // TODO Configure GPIOs
                if ( ( GPIO_Status = GPIO_Configure( Instance->VBUS_Sense, ( GPIO_Configuration_t ) {
                                                                               .Mode = GPIO_Mode_Interrupt,
                                                                               .Function = GPIO_Function_Default, // FIXME Should be GPIO_Function_USB_VSENSE
                                                                               .Pull = GPIO_Pull_None,
                                                                           } ) )
                     != GPIO_Status_Success )
                {
                    Status = USB_Status_Error;
                    break;
                }

                if ( ( GPIO_Status = GPIO_SetOnInterrupt( Instance->VBUS_Sense, ( GPIO_OnInterrupt_t ) { .Callback = GPIO_CallbackOnInterrupt, .Context = Instance } ) ) != GPIO_Status_Success )
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

        Instance->Event = USB_STM32L496VGT6P_Event_None;

        Status = USB_STM32L496VGT6P_SetProcess( USBx, USB_STM32L496VGT6P_ProcessType_Initialize );
    }
    while ( 0 );

    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Cycle( USB_STM32L496VGT6P_t USBx )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;
    USB_STM32L496VGT6P_Status_t STM32L496VGT6P_Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( USBx=%d )", __FUNCTION__, USBx );

        USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USBx ];

        USB_STM32L496VGT6P_Process_t * Process = &Instance->Process;
        USB_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;
        USB_STM32L496VGT6P_Event_t Event = Instance->Event; // CAUTION: Has to copy events occurred at the early start of the cycle, so as to be cleared at the end of the cycle,
                                                            //          which let events occurs after that for the next cycle call

        if ( Operation->Handler != NULL )
        {
            if ( ( STM32L496VGT6P_Status = Operation->Handler( USBx ) ) != USB_STM32L496VGT6P_Status_Success )
            {
                Status = STM32L496VGT6P_Status;
                // FIXME Operation reported non success status, is there any action ?
            }
        }

        if ( Process->Handler != NULL )
        {
            if ( ( STM32L496VGT6P_Status = Process->Handler( USBx ) ) != USB_STM32L496VGT6P_Status_Success )
            {
                Status = STM32L496VGT6P_Status;
                // FIXME Process reported non success status, is there any action ?
            }
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Interrupt ) == USB_STM32L496VGT6P_Event_Interrupt )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_Interrupt;
            USB_Trace( "Interrupt: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Connected ) == USB_STM32L496VGT6P_Event_Connected )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_Connected;
            USB_Debug( "Connected: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Disconnected ) == USB_STM32L496VGT6P_Event_Disconnected )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_Disconnected;
            USB_Debug( "DisConnected: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_StartOfFrame ) == USB_STM32L496VGT6P_Event_StartOfFrame )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_StartOfFrame;
            USB_Debug( "Start Of Frame: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Setup ) == USB_STM32L496VGT6P_Event_Setup )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_Setup;
            USB_Debug( "Setup: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Reset ) == USB_STM32L496VGT6P_Event_Reset )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_Reset;
            USB_Debug( "Reset: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Suspend ) == USB_STM32L496VGT6P_Event_Suspend )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_Suspend;
            USB_Debug( "Suspend: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Resume ) == USB_STM32L496VGT6P_Event_Resume )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_Resume;
            USB_Debug( "Resume: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Tx ) == USB_STM32L496VGT6P_Event_Tx )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_Tx;
            USB_Debug( "TX InProgress: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Rx ) == USB_STM32L496VGT6P_Event_Rx )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_Rx;
            USB_Debug( "RX InProgress: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_TxComplete ) == USB_STM32L496VGT6P_Event_TxComplete )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_TxComplete;
            USB_Debug( "TX Complete: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_RxComplete ) == USB_STM32L496VGT6P_Event_RxComplete )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_RxComplete;
            USB_Debug( "RX Complete: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_TxIncomplete ) == USB_STM32L496VGT6P_Event_TxIncomplete )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_TxIncomplete;
            USB_Debug( "TX Incomplete: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_RxIncomplete ) == USB_STM32L496VGT6P_Event_RxIncomplete )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_RxIncomplete;
            USB_Debug( "RX Incomplete: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_DataTerminalReady_Enabled ) == USB_STM32L496VGT6P_Event_DataTerminalReady_Enabled )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_DataTerminalReady_Enabled;
            USB_Debug( "DTR Enabled: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_DataTerminalReady_Disabled ) == USB_STM32L496VGT6P_Event_DataTerminalReady_Disabled )
        {
            Instance->Event &= ~USB_STM32L496VGT6P_Event_DataTerminalReady_Disabled;
            USB_Debug( "DTR Disabled: USBx=%d", USBx );
            // TODO Invoke Callback
        }

        for ( USB_STM32L496VGT6P_Interface_t Interface = USB_STM32L496VGT6P_Interface_1; Interface < USB_STM32L496VGT6P_Interface_Count; ++Interface )
        {
            USB_STM32L496VGT6P_InterfaceContext_t * InterfaceContext = &Instance->Interface[ Interface ];
            BUFFER_t * Transmit = &InterfaceContext->Transmit;

            if ( Transmit->Length < 1 || InterfaceContext->DataTerminalReady != USB_STM32L496VGT6P_DataTerminalReady_Enabled )
            {
                continue;
            }

            USBD_StatusTypeDef USBD_Status = USBD_OK;
            if ( ( USBD_Status = CDC_Transmit_FS( Transmit->Content, Transmit->Length, Interface ) ) != USBD_OK )
            {
                Status = USB_STM32L496VGT6P_Status_Error;
            }
        }
    }
    while ( 0 );

    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_DeInitialize( USB_STM32L496VGT6P_t USBx )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;
    do
    {
        USB_Trace( "%s( USBx=%d )", __FUNCTION__, USBx );

        USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USBx ];

        UTIL_UNUSED( Instance );

        // FIXME
        Status = USB_STM32L496VGT6P_Status_NotSupported;
    }
    while ( 0 );
    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_SetProcess( USB_STM32L496VGT6P_t USBx, USB_STM32L496VGT6P_ProcessType_t ProcessType )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( USBx=%d, ProcessType=%d )", __FUNCTION__, USBx, ProcessType );

        USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Instance->Process;
        USB_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        switch ( ProcessType )
        {
            case USB_STM32L496VGT6P_ProcessType_None:
                Process->Handler = NULL;
                break;

            case USB_STM32L496VGT6P_ProcessType_Initialize:
                Process->Handler = USB_STM32L496VGT6P_ProcessInitialize;
                break;

            default:
                USB_Warning( "%s Not Handled Type %d", __FUNCTION__, ProcessType );
                Status = USB_STM32L496VGT6P_Status_NotSupported;
                break;
        }
        if ( Status != USB_STM32L496VGT6P_Status_Success )
        {
            break;
        }

        Process->Type = ProcessType;

        Operation->Handler = NULL;
        Operation->Status = USB_STM32L496VGT6P_Status_Success;
        Operation->Timeout = USB_STM32L496VGT6P_Context.Timestamp;

        switch ( ProcessType )
        {
            case USB_STM32L496VGT6P_ProcessType_None:
                Operation->Type = USB_STM32L496VGT6P_OperationType_None;
                break;

            default:
                Operation->Type = USB_STM32L496VGT6P_OperationType_Pending;
                break;
        }
    }
    while ( 0 );

    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_ProcessInitialize( USB_STM32L496VGT6P_t USBx )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( USBx=%d )", __FUNCTION__, USBx );

        USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Instance->Process;
        USB_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        if ( Process->Type != USB_STM32L496VGT6P_ProcessType_Initialize )
        {
            USB_Error( "%s Got %d Expected %d", __FUNCTION__, Process->Type, USB_STM32L496VGT6P_ProcessType_Initialize );
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }

        if ( Operation->Handler != NULL )
        {
            // Operation In-progress
            break;
        }

        if ( Operation->Status != USB_STM32L496VGT6P_Status_Success )
        {
            Operation->Type = USB_STM32L496VGT6P_OperationType_None;
        }

        switch ( Operation->Type )
        {
            case USB_STM32L496VGT6P_OperationType_Pending:
                Operation->Status = USB_STM32L496VGT6P_OperationCommitExecute( USBx );
                break;

            case USB_STM32L496VGT6P_OperationType_Commit:
            default:
                // TODO Invoke Callback
                // if ( Instance->OnComplete != NULL )
                // {
                //     Instance->OnComplete( USBx, Operation->Status );
                // }
                Status = USB_STM32L496VGT6P_SetProcess( USBx, USB_STM32L496VGT6P_ProcessType_None );
                break;
        }
    }
    while ( 0 );

    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationCommitExecute( USB_STM32L496VGT6P_t USBx )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( USBx=%d )", __FUNCTION__, USBx );

        USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Instance->Process;
        USB_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

    // FIXME Keep CubeMX generated configurations as is for now
    #if 0
      HAL_StatusTypeDef HAL_Status = HAL_ERROR;
      if ( ( HAL_Status = HAL_USB_Init( &Instance->Context->USBx ) ) != HAL_OK )
      {
        Status = USB_STM32L496VGT6P_Status_Error;
        break;
      }
    #endif

        Operation->Type = USB_STM32L496VGT6P_OperationType_Commit;
        Operation->Handler = USB_STM32L496VGT6P_OperationCommitResolve;
        Operation->Status = USB_STM32L496VGT6P_Status_Success;
        Operation->Timeout = USB_STM32L496VGT6P_Context.Timestamp;

        TIM_Status_t TIM_Status = TIM_Status_Error;
        if ( ( TIM_Status = TIM_Timestamp_AddMillisecond( &Operation->Timeout, 0 ) ) != TIM_Status_Success )
        {
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }
    }
    while ( 0 );

    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationCommitResolve( USB_STM32L496VGT6P_t USBx )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( USBx=%d )", __FUNCTION__, USBx );

        USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Instance->Process;
        USB_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        if ( Operation->Type != USB_STM32L496VGT6P_OperationType_Commit )
        {
            USB_Error( "%s Got %d Expected %d", __FUNCTION__, Operation->Type, USB_STM32L496VGT6P_OperationType_Commit );
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }

        TIM_Status_t TIM_Status = TIM_Status_Error;
        if ( ( TIM_Status = TIM_IsExpiredTimestamp( USB_TIM, &Operation->Timeout ) ) == TIM_Status_Success )
        {
            Operation->Status = USB_STM32L496VGT6P_Status_Success;
            Operation->Handler = NULL;
            break;
        }
    }
    while ( 0 );

    return Status;
}

// #############################################################################
// #### Public Method(s) #######################################################
// #############################################################################

USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Initialize( USB_STM32L496VGT6P_t USBx )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( USBx=%d )", __FUNCTION__, USBx );

        if ( ( Status = USB_STM32L496VGT6P_Context_Initialize( ) ) != USB_STM32L496VGT6P_Status_Success )
        {
            break;
        }

        if ( ( Status = USB_STM32L496VGT6P_Instance_Initialize( USBx ) ) != USB_STM32L496VGT6P_Status_Success )
        {
            break;
        }
    }
    while ( 0 );

    return Status;
}

USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Cycle( USB_STM32L496VGT6P_t USBx )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( USBx=%d )", __FUNCTION__, USBx );

        if ( ( Status = USB_STM32L496VGT6P_Context_Cycle( ) ) != USB_STM32L496VGT6P_Status_Success )
        {
            break;
        }

        if ( ( Status = USB_STM32L496VGT6P_Instance_Cycle( USBx ) ) != USB_STM32L496VGT6P_Status_Success )
        {
            break;
        }
    }
    while ( 0 );

    return Status;
}

USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_DeInitialize( USB_STM32L496VGT6P_t USBx )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( USBx=%d )", __FUNCTION__, USBx );

        if ( ( Status = USB_STM32L496VGT6P_Instance_DeInitialize( USBx ) ) != USB_STM32L496VGT6P_Status_Success )
        {
            break;
        }

        if ( ( Status = USB_STM32L496VGT6P_Context_DeInitialize( ) ) != USB_STM32L496VGT6P_Status_Success )
        {
            break;
        }
    }
    while ( 0 );

    return Status;
}

USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_IsReady( USB_STM32L496VGT6P_t USBx, USB_STM32L496VGT6P_Interface_t Interface )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( USBx=%d )", __FUNCTION__, USBx );

        USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USBx ];

        // FIXME
        // if ( Process->Type != USB_STM32L496VGT6P_ProcessType_None
        //      || Operation->Type != USB_STM32L496VGT6P_OperationType_None )
        // {
        //     Status = USB_STM32L496VGT6P_Status_Busy;
        //     break;
        // }

        if ( Instance->Interface[ Interface ].DataTerminalReady != USB_STM32L496VGT6P_DataTerminalReady_Enabled )
        {
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }
    }
    while ( 0 );

    return Status;
}

USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Write( USB_STM32L496VGT6P_t USBx, USB_STM32L496VGT6P_Interface_t Interface, USB_STM32L496VGT6P_Data_t * Data, USB_STM32L496VGT6P_DataLength_t DataLength )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;
    BUFFER_Status_t BUFFER_Status = BUFFER_Status_Success;

    do
    {
        USB_Trace( "%s( USBx=%d, Interface=%d, Data=%p, Length=%d )", __FUNCTION__, USBx, Interface, Data, DataLength );

        USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USBx ];

        if ( ( BUFFER_Status = BUFFER_PushMemoryBack( &Instance->Interface[ Interface ].Transmit, Data, DataLength ) ) != BUFFER_Status_Success )
        {
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }
    }
    while ( 0 );
    return Status;
}

USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Read( USB_STM32L496VGT6P_t USBx, USB_STM32L496VGT6P_Interface_t Interface, USB_STM32L496VGT6P_Data_t * Data, USB_STM32L496VGT6P_DataLength_t DataLength )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;
    BUFFER_Status_t BUFFER_Status = BUFFER_Status_Success;

    do
    {
        USB_Trace( "%s( USBx=%d, Interface=%d, Data=%p, Length=%d )", __FUNCTION__, USBx, Interface, Data, DataLength );

        USB_STM32L496VGT6P_Instance_t * Instance = &USB_STM32L496VGT6P_Context.Instance[ USBx ];

        if ( ( BUFFER_Status = BUFFER_PullMemoryFrontCircular( &Instance->Interface[ Interface ].Receive, Data, DataLength ) ) != BUFFER_Status_Success )
        {
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }
    }
    while ( 0 );
    return Status;
}

// #############################################################################
// #### Public Variable(s) #####################################################
// #############################################################################

const char USB_STM32L496VGT6P_VERSION[] = "0.0.0.v20260604-1610";

// #############################################################################
// #### File Guard #############################################################
// #############################################################################

#endif /* STM32L496xx */

// #############################################################################
// #### END OF FILE ############################################################
// #############################################################################
