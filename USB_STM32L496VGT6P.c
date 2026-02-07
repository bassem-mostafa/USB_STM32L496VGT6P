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
    USB_STM32L496VGT6P_OperationType_None = 0,          ///< None
    USB_STM32L496VGT6P_OperationType_Pending,           ///< Pending
    USB_STM32L496VGT6P_OperationType_Commit,            ///< Commit
    USB_STM32L496VGT6P_OperationType_DataTerminalReady, ///< Data Terminal Ready (DTR)
    USB_STM32L496VGT6P_OperationType_Transmit,          ///< Transmit
    USB_STM32L496VGT6P_OperationType_Receive,           ///< Receive
} USB_STM32L496VGT6P_OperationType_t;

/**
 * @brief USB STM32L496VGT6P Operation Handler
 */
typedef USB_STM32L496VGT6P_Status_t ( *USB_STM32L496VGT6P_OperationHandler_t )( USB_STM32L496VGT6P_Instance_t * Instance );

/**
 * @brief USB STM32L496VGT6P Operation Context
 *
 * @struct USB_STM32L496VGT6P_OperationContext_t
 */
typedef struct USB_STM32L496VGT6P_OperationContext
{
    USB_STM32L496VGT6P_Data_t * DataTx;
    USB_STM32L496VGT6P_DataLength_t DataTxLength;

    USB_STM32L496VGT6P_Data_t * DataRx;
    USB_STM32L496VGT6P_DataLength_t DataRxLength;
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
    USB_STM32L496VGT6P_ProcessType_Transmit,   ///< Transmit
} USB_STM32L496VGT6P_ProcessType_t;

/**
 * @brief USB STM32L496VGT6P Process Handler
 */
typedef USB_STM32L496VGT6P_Status_t ( *USB_STM32L496VGT6P_ProcessHandler_t )( USB_STM32L496VGT6P_Instance_t * Instance );

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

typedef struct USB_STM32L496VGT6P_Instance_Context
{
    PCD_HandleTypeDef USBx;

    USB_STM32L496VGT6P_DataTerminalReady_t DataTerminalReady;

    USB_STM32L496VGT6P_Event_t Event;

    USB_STM32L496VGT6P_Process_t Process;
} USB_STM32L496VGT6P_Instance_Context_t;

typedef struct USB_STM32L496VGT6P_Context
{
    TIM_Timestamp_t Timestamp;
    USB_STM32L496VGT6P_Instance_Context_t Context[ USB_STM32L496VGT6P_Count ];
} USB_STM32L496VGT6P_Context_t;

// #############################################################################
// #### Private Method(s) Prototype ############################################
// #############################################################################

static GPIO_Status_t GPIO_CallbackOnInterrupt( GPIO_t GPIOx, GPIO_ContextOnInterrupt_t * Context );

void USB_STM32L496VGT6P_RxCpltCallback( uint8_t * pbuf, uint32_t * Len );
void USB_STM32L496VGT6P_TxCpltCallback( uint8_t * pbuf, uint32_t * Len );

void USB_STM32L496VGT6P_DataTerminalReadyCallback( uint8_t dtr );

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

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Initialize( USB_STM32L496VGT6P_Instance_t * Instance );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Cycle( USB_STM32L496VGT6P_Instance_t * Instance );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_DeInitialize( USB_STM32L496VGT6P_Instance_t * Instance );

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_SetProcess( USB_STM32L496VGT6P_Instance_t * Instance, USB_STM32L496VGT6P_ProcessType_t ProcessType );

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_ProcessInitialize( USB_STM32L496VGT6P_Instance_t * Instance );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_ProcessTransmit( USB_STM32L496VGT6P_Instance_t * Instance );

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationCommitExecute( USB_STM32L496VGT6P_Instance_t * Instance );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationCommitResolve( USB_STM32L496VGT6P_Instance_t * Instance );

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationDataTerminalReadyExecute( USB_STM32L496VGT6P_Instance_t * Instance );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationDataTerminalReadyResolve( USB_STM32L496VGT6P_Instance_t * Instance );

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationTransmitExecute( USB_STM32L496VGT6P_Instance_t * Instance );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationTransmitResolve( USB_STM32L496VGT6P_Instance_t * Instance );

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationReceiveExecute( USB_STM32L496VGT6P_Instance_t * Instance );
static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationReceiveResolve( USB_STM32L496VGT6P_Instance_t * Instance );

// #############################################################################
// #### Private Variable(s) ####################################################
// #############################################################################

static USB_STM32L496VGT6P_Context_t USB_STM32L496VGT6P_Context;

// #############################################################################
// #### Private Method(s) ######################################################
// #############################################################################

static GPIO_Status_t GPIO_CallbackOnInterrupt( GPIO_t GPIOx, GPIO_ContextOnInterrupt_t * Context )
{
    GPIO_Status_t GPIO_Status = GPIO_Status_Success;

    do
    {
        USB_Debug( "%s%s( GPIOx=%d, Context=%p )", UTIL_StringConcatenateConstant( UTIL_CSI_SelectGraphicRendition( UTIL_CSI_SelectGraphicRenditionColorForegroundBlack ), UTIL_CSI_SelectGraphicRendition( UTIL_CSI_SelectGraphicRenditionColorBackgroundCyan ) ), __FUNCTION__, GPIOx, Context );

        USB_STM32L496VGT6P_Instance_t * Instance = Context;
        if ( GPIOx == Instance->VBUS_Sense )
        {
            /*
             * @brief  Handle USB VBUS detection upon external interrupt
             */
            PCD_HandleTypeDef * hpcd = &Instance->Context->USBx;
            HAL_PCDEx_BCD_VBUSDetect( hpcd );
        }
    }
    while ( 0 );

    return GPIO_Status;
}

void USB_STM32L496VGT6P_RxCpltCallback( uint8_t * pbuf, uint32_t * Len )
{
    USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ];
    USB_STM32L496VGT6P_Process_t * Process = &Context->Process;
    USB_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

    Context->Event |= USB_STM32L496VGT6P_Event_RxComplete;

    if ( Operation->Context.DataRxLength > *Len && Operation->Context.DataRx != NULL )
    {
        UTIL_MemoryCopy( Operation->Context.DataRx, pbuf, *Len );
        Operation->Context.DataRxLength -= *Len;
        Operation->Context.DataRx += *Len;
    }

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
    USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ];

    Context->Event |= USB_STM32L496VGT6P_Event_TxComplete;

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

void USB_STM32L496VGT6P_DataTerminalReadyCallback( uint8_t dtr )
{
    USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ];

    if ( dtr )
    {
        Context->Event |= USB_STM32L496VGT6P_Event_DataTerminalReady_Enabled;
    }
    else
    {
        Context->Event |= USB_STM32L496VGT6P_Event_DataTerminalReady_Disabled;
    }
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
        // FIXME Instead of using SystemClockConfig_Resume();
        //       Use directly SystemClock_Config() through KERNEL;
        KERNEL_ClockEnable( );
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

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Context_Initialize( void )
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

        // FIXME Remove the usage of `MX_USB_DEVICE_Init()`
    #if 1
        extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
        extern void MX_USB_DEVICE_Init( void );
        MX_USB_DEVICE_Init( );
        USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ].USBx = hpcd_USB_OTG_FS;
        // FIX Let the USBD use our instance
        ( ( USBD_HandleTypeDef * ) ( hpcd_USB_OTG_FS.pData ) )->pData = &USB_STM32L496VGT6P_Context.Context[ USB_STM32L496VGT6P_1 ].USBx;
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
    }
    while ( 0 );

    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Initialize( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;
    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( Instance == NULL )
        {
            Status = USB_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ Instance->USBx ];

        switch ( Instance->USBx )
        {
            case USB_STM32L496VGT6P_1:
                GPIO_Status_t GPIO_Status = GPIO_Status_Success;

                // TODO Configure GPIOs

                if ( ( GPIO_Status = GPIO_SetMode( Instance->VBUS_Sense, GPIO_Mode_Interrupt ) ) != GPIO_Status_Success )
                {
                    Status = USB_Status_Error;
                    break;
                }

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

        Context->Event = USB_STM32L496VGT6P_Event_None;

        Instance->Context = Context;

        Status = USB_STM32L496VGT6P_SetProcess( Instance, USB_STM32L496VGT6P_ProcessType_Initialize );
    }
    while ( 0 );
    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_Cycle( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;
    USB_STM32L496VGT6P_Status_t STM32L496VGT6P_Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( Instance == NULL )
        {
            Status = USB_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ Instance->USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Context->Process;
        USB_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;
        USB_STM32L496VGT6P_Event_t Event = Context->Event; // CAUTION: Has to copy events occurred at the early start of the cycle, so as to be cleared at the end of the cycle,
                                                           //          which let events occurs after that for the next cycle call

        if ( Operation->Handler != NULL )
        {
            if ( ( STM32L496VGT6P_Status = Operation->Handler( Instance ) ) != USB_STM32L496VGT6P_Status_Success )
            {
                Status = STM32L496VGT6P_Status;
                // FIXME Operation reported non success status, is there any action ?
            }
        }

        if ( Process->Handler != NULL )
        {
            if ( ( STM32L496VGT6P_Status = Process->Handler( Instance ) ) != USB_STM32L496VGT6P_Status_Success )
            {
                Status = STM32L496VGT6P_Status;
                // FIXME Process reported non success status, is there any action ?
            }
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Interrupt ) == USB_STM32L496VGT6P_Event_Interrupt )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_Interrupt;
            USB_Trace( "Interrupt: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Connected ) == USB_STM32L496VGT6P_Event_Connected )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_Connected;
            USB_Debug( "Connected: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Disconnected ) == USB_STM32L496VGT6P_Event_Disconnected )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_Disconnected;
            USB_Debug( "DisConnected: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_StartOfFrame ) == USB_STM32L496VGT6P_Event_StartOfFrame )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_StartOfFrame;
            USB_Debug( "Start Of Frame: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Setup ) == USB_STM32L496VGT6P_Event_Setup )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_Setup;
            USB_Debug( "Setup: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Reset ) == USB_STM32L496VGT6P_Event_Reset )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_Reset;
            USB_Debug( "Reset: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Suspend ) == USB_STM32L496VGT6P_Event_Suspend )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_Suspend;
            USB_Debug( "Suspend: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Resume ) == USB_STM32L496VGT6P_Event_Resume )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_Resume;
            USB_Debug( "Resume: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Tx ) == USB_STM32L496VGT6P_Event_Tx )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_Tx;
            USB_Debug( "TX InProgress: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_Rx ) == USB_STM32L496VGT6P_Event_Rx )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_Rx;
            USB_Debug( "RX InProgress: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_TxComplete ) == USB_STM32L496VGT6P_Event_TxComplete )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_TxComplete;
            USB_Debug( "TX Complete: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_RxComplete ) == USB_STM32L496VGT6P_Event_RxComplete )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_RxComplete;
            USB_Debug( "RX Complete: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_TxIncomplete ) == USB_STM32L496VGT6P_Event_TxIncomplete )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_TxIncomplete;
            USB_Debug( "TX Incomplete: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_RxIncomplete ) == USB_STM32L496VGT6P_Event_RxIncomplete )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_RxIncomplete;
            USB_Debug( "RX Incomplete: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_DataTerminalReady_Enabled ) == USB_STM32L496VGT6P_Event_DataTerminalReady_Enabled )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_DataTerminalReady_Enabled;
            USB_Debug( "DTR Enabled: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback

            Context->DataTerminalReady = USB_STM32L496VGT6P_DataTerminalReady_Enabled;
        }

        if ( ( Event & USB_STM32L496VGT6P_Event_DataTerminalReady_Disabled ) == USB_STM32L496VGT6P_Event_DataTerminalReady_Disabled )
        {
            Context->Event &= ~USB_STM32L496VGT6P_Event_DataTerminalReady_Disabled;
            USB_Debug( "DTR Disabled: Instance=%p, USBx=%d", Instance, Instance->USBx );
            // TODO Invoke Callback

            Context->DataTerminalReady = USB_STM32L496VGT6P_DataTerminalReady_Disabled;
        }
    }
    while ( 0 );
    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Instance_DeInitialize( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;
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

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_SetProcess( USB_STM32L496VGT6P_Instance_t * Instance, USB_STM32L496VGT6P_ProcessType_t ProcessType )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( Instance=%p, ProcessType=%d )", __FUNCTION__, Instance, ProcessType );

        USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ Instance->USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Context->Process;
        USB_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        switch ( ProcessType )
        {
            case USB_STM32L496VGT6P_ProcessType_None:
                Process->Handler = NULL;
                break;

            case USB_STM32L496VGT6P_ProcessType_Initialize:
                Process->Handler = USB_STM32L496VGT6P_ProcessInitialize;
                break;

            case USB_STM32L496VGT6P_ProcessType_Transmit:
                Process->Handler = USB_STM32L496VGT6P_ProcessTransmit;
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

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_ProcessInitialize( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ Instance->USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Context->Process;
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
                Operation->Status = USB_STM32L496VGT6P_OperationCommitExecute( Instance );
                break;

            case USB_STM32L496VGT6P_OperationType_Commit:
            default:
                // TODO Invoke Callback
                // if ( Instance->OnComplete != NULL )
                // {
                //     Instance->OnComplete( Instance, Operation->Status );
                // }
                Status = USB_STM32L496VGT6P_SetProcess( Instance, USB_STM32L496VGT6P_ProcessType_None );
                break;
        }
    }
    while ( 0 );

    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_ProcessTransmit( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ Instance->USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Context->Process;
        USB_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        if ( Process->Type != USB_STM32L496VGT6P_ProcessType_Transmit )
        {
            USB_Error( "%s Got %d Expected %d", __FUNCTION__, Process->Type, USB_STM32L496VGT6P_ProcessType_Transmit );
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
                Operation->Status = USB_STM32L496VGT6P_OperationTransmitExecute( Instance );
                break;

            case USB_STM32L496VGT6P_OperationType_Transmit:
            default:
                // TODO Invoke Callback
                // if ( Instance->OnComplete != NULL )
                // {
                //     Instance->OnComplete( Instance, Operation->Status );
                // }
                Status = USB_STM32L496VGT6P_SetProcess( Instance, USB_STM32L496VGT6P_ProcessType_None );
                break;
        }
    }
    while ( 0 );

    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationCommitExecute( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ Instance->USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Context->Process;
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

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationCommitResolve( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ Instance->USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Context->Process;
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

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationTransmitExecute( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ Instance->USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Context->Process;
        USB_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        USBD_StatusTypeDef USBD_Status = USBD_OK;
        if ( ( USBD_Status = CDC_Transmit_FS( Operation->Context.DataTx, Operation->Context.DataTxLength ) ) != USBD_OK )
        {
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }

        Operation->Type = USB_STM32L496VGT6P_OperationType_Transmit;
        Operation->Handler = USB_STM32L496VGT6P_OperationTransmitResolve;
        Operation->Status = USB_STM32L496VGT6P_Status_Success;
        Operation->Timeout = USB_STM32L496VGT6P_Context.Timestamp;

        TIM_Status_t TIM_Status = TIM_Status_Error;
        if ( ( TIM_Status = TIM_Timestamp_AddMillisecond( &Operation->Timeout, 1000 ) ) != TIM_Status_Success )
        {
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }
    }
    while ( 0 );

    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationTransmitResolve( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ Instance->USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Context->Process;
        USB_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        if ( Operation->Type != USB_STM32L496VGT6P_OperationType_Transmit )
        {
            USB_Error( "%s Got %d Expected %d", __FUNCTION__, Operation->Type, USB_STM32L496VGT6P_OperationType_Transmit );
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }

        TIM_Status_t TIM_Status = TIM_Status_Error;
        if ( ( TIM_Status = TIM_IsExpiredTimestamp( USB_TIM, &Operation->Timeout ) ) == TIM_Status_Success )
        {
            Operation->Status = USB_STM32L496VGT6P_Status_Timeout;
            Operation->Handler = NULL;
            break;
        }

        if ( ( Context->Event & USB_STM32L496VGT6P_Event_TxComplete ) != 0 )
        {
            Operation->Status = USB_STM32L496VGT6P_Status_Success;
            Operation->Handler = NULL;
        }
    }
    while ( 0 );

    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationReceiveExecute( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ Instance->USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Context->Process;
        USB_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

    #if 0 // FIXME
        HAL_StatusTypeDef HAL_Status = HAL_ERROR;
        if ( ( HAL_Status = HAL_USB_Receive_DMA( &Instance->Context->USBx, Operation->Context.DataRx, Operation->Context.DataRxLength ) ) != HAL_OK )
        {
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }
    #endif

        Operation->Type = USB_STM32L496VGT6P_OperationType_Receive;
        Operation->Handler = USB_STM32L496VGT6P_OperationReceiveResolve;
        Operation->Status = USB_STM32L496VGT6P_Status_Success;
        Operation->Timeout = USB_STM32L496VGT6P_Context.Timestamp;

        TIM_Status_t TIM_Status = TIM_Status_Error;
        if ( ( TIM_Status = TIM_Timestamp_AddMillisecond( &Operation->Timeout, 1000 ) ) != TIM_Status_Success )
        {
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }
    }
    while ( 0 );

    return Status;
}

static USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_OperationReceiveResolve( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ Instance->USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Context->Process;
        USB_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        if ( Operation->Type != USB_STM32L496VGT6P_OperationType_Receive )
        {
            USB_Error( "%s Got %d Expected %d", __FUNCTION__, Operation->Type, USB_STM32L496VGT6P_OperationType_Receive );
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }

        TIM_Status_t TIM_Status = TIM_Status_Error;
        if ( ( TIM_Status = TIM_IsExpiredTimestamp( USB_TIM, &Operation->Timeout ) ) == TIM_Status_Success )
        {
            Operation->Status = USB_STM32L496VGT6P_Status_Timeout;
            Operation->Handler = NULL;
            break;
        }

        if ( ( Context->Event & USB_STM32L496VGT6P_Event_RxComplete ) != 0 )
        {
            Operation->Status = USB_STM32L496VGT6P_Status_Success;
            Operation->Handler = NULL;
        }
    }
    while ( 0 );

    return Status;
}

// #############################################################################
// #### Public Method(s) #######################################################
// #############################################################################

USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Initialize( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;
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
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;
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
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;
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

USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_IsReady( USB_STM32L496VGT6P_Instance_t * Instance )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;

    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( Instance == NULL )
        {
            Status = USB_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        if ( Instance->Context == NULL )
        {
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }

        USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ Instance->USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Context->Process;
        USB_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        if ( Process->Type != USB_STM32L496VGT6P_ProcessType_None
             || Operation->Type != USB_STM32L496VGT6P_OperationType_None )
        {
            Status = USB_STM32L496VGT6P_Status_Busy;
            break;
        }

        if ( Context->DataTerminalReady != USB_STM32L496VGT6P_DataTerminalReady_Enabled )
        {
            Status = USB_STM32L496VGT6P_Status_Error;
            break;
        }
    }
    while ( 0 );

    return Status;
}

USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Write( USB_STM32L496VGT6P_Instance_t * Instance, USB_STM32L496VGT6P_Data_t * Data, USB_STM32L496VGT6P_DataLength_t DataLength )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;
    do
    {
        USB_Trace( "%s( Instance=%p, Data=%p, Length=%d )", __FUNCTION__, Instance, Data, DataLength );

        if ( Instance == NULL )
        {
            Status = USB_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        USB_STM32L496VGT6P_Instance_Context_t * Context = &USB_STM32L496VGT6P_Context.Context[ Instance->USBx ];
        USB_STM32L496VGT6P_Process_t * Process = &Context->Process;
        USB_STM32L496VGT6P_Operation_t * Operation = &Process->Context.Operation;

        if ( Process->Type != USB_STM32L496VGT6P_ProcessType_None
             || Operation->Type != USB_STM32L496VGT6P_OperationType_None )
        {
            Status = USB_STM32L496VGT6P_Status_Busy;
            break;
        }

        Status = USB_STM32L496VGT6P_SetProcess( Instance, USB_STM32L496VGT6P_ProcessType_Transmit );

        Operation->Context.DataTx = Data;
        Operation->Context.DataTxLength = DataLength;
    }
    while ( 0 );
    return Status;
}

USB_STM32L496VGT6P_Status_t USB_STM32L496VGT6P_Read( USB_STM32L496VGT6P_Instance_t * Instance, USB_STM32L496VGT6P_Data_t * Data, USB_STM32L496VGT6P_DataLength_t DataLength )
{
    USB_STM32L496VGT6P_Status_t Status = USB_STM32L496VGT6P_Status_Success;
    do
    {
        USB_Trace( "%s( Instance=%p )", __FUNCTION__, Instance );

        if ( Instance == NULL )
        {
            Status = USB_STM32L496VGT6P_Status_ArgumentInvalid;
            break;
        }

        // FIXME Support Reading
        Status = USB_STM32L496VGT6P_Status_NotSupported;
    }
    while ( 0 );
    return Status;
}

// #############################################################################
// #### Public Variable(s) #####################################################
// #############################################################################

const char USB_STM32L496VGT6P_VERSION[] = "0.0.0.v20260207-2231";

// #############################################################################
// #### File Guard #############################################################
// #############################################################################

#endif /* STM32L496xx */

// #############################################################################
// #### END OF FILE ############################################################
// #############################################################################
