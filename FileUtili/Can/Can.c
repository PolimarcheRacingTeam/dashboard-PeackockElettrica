/* ***********************************************************************
 * Includes
 * ***********************************************************************/

#include "Can.h"

/* ***********************************************************************
 * Private Defines
 * ***********************************************************************/

/* ***********************************************************************
 * Global Variables
 * ***********************************************************************/

uCAN_MSG txMessage, rxMessage;       // Message structures for transmission and reception

Vector accelData = {0};              // Vector to store accelerometer data
Vector gyroData = {0};               // Vector to store gyroscope data

CAN_StatusFlags canStatusFlags;      // Status flags for CAN messages


/* ***********************************************************************
 * Functions
 * ***********************************************************************/

/* ***********************************************************************
 * CAN INITIALIZATION
 * Initializes the CAN communication settings, configures filters, and
 * activates notifications for received messages.
 *
 * This function must be called once at startup to ensure proper
 * operation of the CAN interface.
 *************************************************************************/
void CanInit(void) {
    /* Initialize CAN communication settings */

	// Configuring CAN filter parameters
	CAN_FilterTypeDef sFilterConfig;

	// Use filter bank 0 for this filter configuration
	// Filter banks are memory locations where filter settings are stored.
	// The STM32F466RE has a maximum of 28 filter banks.
	// Each filter bank can hold one or more filters. Bank 0 is the first of these banks.
	sFilterConfig.FilterBank = 0;

	// Set the filter to operate in identifier mask mode
	// This mode allows filtering based on specific bits in the CAN ID.
	// The filter ID and mask determine which CAN messages are accepted.
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;

	// Set the filter to 32-bit scale
	// This means the filter will use a single 32-bit filter instead of two 16-bit filters.
	// A 32-bit filter can match a full 29-bit extended CAN ID or two 11-bit standard CAN IDs.
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;

	// Set the high part of the filter ID to 0x0000
	// This is the upper 16 bits of the 32-bit filter ID.
	// It is used to match the incoming CAN messages.
	sFilterConfig.FilterIdHigh = 0x0000;

	// Set the low part of the filter ID to 0x0000
	// This is the lower 16 bits of the 32-bit filter ID.
	// Together with FilterIdHigh, it forms the full 32-bit ID to match incoming messages.
	sFilterConfig.FilterIdLow = 0x0000;

	// Set the high part of the filter mask to 0x0000
	// This is the upper 16 bits of the mask used to determine which bits in the ID are significant.
	// A mask bit set to 0 means the corresponding ID bit is "don't care".
	sFilterConfig.FilterMaskIdHigh = 0x0000;

	// Set the low part of the filter mask to 0x0000
	// This is the lower 16 bits of the mask.
	// Together with FilterMaskIdHigh, it forms the full 32-bit mask.
	// A mask of 0x0000 means all bits are "don't care" and will accept all IDs.
	sFilterConfig.FilterMaskIdLow = 0x0000;

	// Assign this filter to CAN receive FIFO 0
	// Incoming messages that pass this filter will be stored in FIFO 0.
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;

	// Enable this filter configuration
	// Activates the filter so that it starts processing incoming messages.
	sFilterConfig.FilterActivation = ENABLE;

	// Configure the start filter bank for the slave CAN controller (CAN2) to filter bank 14.
	// The STM32F4 microcontroller has a total of 28 filter banks available for CAN message filtering.
	// These filter banks can be shared between the two CAN controllers: CAN1 (master) and CAN2 (slave).
	// By setting sFilterConfig.SlaveStartFilterBank to 14, the first 14 filter banks (0 to 13) are allocated to CAN1,
	// and the remaining 14 filter banks (14 to 27) are allocated to CAN2.
	// This division allows each CAN controller to have its own set of filters for incoming CAN messages.
	// Adjusting this value allows for flexible distribution of the available filter banks based on the application's requirements.
	sFilterConfig.SlaveStartFilterBank = 14;

    // Configure the CAN filter
    if (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) {
        Error_Handler();                                               // Handle configuration error
    }

    HAL_CAN_Start(&hcan1);                                             // Start CAN communication
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING); // Activate notifications for RX FIFO

    int initResult = CANSPI_Initialize();                              // Initialize CAN SPI
    if (initResult < 0) {
        // TODO: Handle initialization error
    }
}

/* ***********************************************************************
 * CAN TRANSMISSION
 * Transmits a CAN message based on the specified parameters and selected
 * transceiver type. This function abstracts the transmission process,
 * supporting different transceiver implementations.
 *
 * Use the appropriate transmission function depending on the defined
 * transceiver macro (USE_SN65HVD230 or MCP2515).
 *************************************************************************/
void Transmit_CAN_Message(CAN_HandleTypeDef *hcan, uint32_t StdId, uint32_t DLC, uint8_t *TxData) {
    /* Transmit CAN message based on the selected transceiver */
    #ifdef USE_SN65HVD230
        Transmit_CAN_Message_SN65HVD230(hcan, StdId, DLC, TxData);                // Use SN65HVD230 transceiver
    #else
        Transmit_CAN_Message_MCP2515(StdId, DLC, TxData);                         // Use MCP2515 transceiver
    #endif
}

void Transmit_CAN_Message_SN65HVD230(CAN_HandleTypeDef *hcan, uint32_t StdId, uint32_t DLC, uint8_t *TxData)
{
	// Initialize CAN header
	CAN_TxHeaderTypeDef TxHeader;

	// Standard 11-bit ID
	TxHeader.StdId = StdId; // Use TxHeader.ExtId = StdId; for 29-bit extended IDs

	// Configuring a CAN message with standard ID
	TxHeader.IDE = CAN_ID_STD;

	// Request type: Data frame
	TxHeader.RTR = CAN_RTR_DATA;

	// Number of bytes sent (maximum 8 bytes)
	uint8_t maxLength = (DLC < 8) ? DLC : 8;
	TxHeader.DLC = maxLength;

	// Select Tx Mailbox
	uint32_t TxMailbox = CAN_TX_MAILBOX0;

	// Try to add the message to the CAN bus
	if (HAL_CAN_AddTxMessage(hcan, &TxHeader, TxData, &TxMailbox) == HAL_OK) {
		// Turn on the built-in LED to indicate successful transmission
		// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	} else {
		// Turn off the built-in LED to indicate an error during transmission
		// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		Error_Handler(); // Handle error if message transmission fails
	}
}

void Transmit_CAN_Message_MCP2515(uint32_t StdId, uint8_t DLC, uint8_t *TxData)
{
	// Configuring a CAN message with standard ID
	txMessage.frame.idType = dSTANDARD_CAN_MSG_ID_2_0B; // Use dEXTENDED_CAN_MSG_ID_2_0B for 29-bit extended IDs

	// Standard 11-bit ID
	txMessage.frame.id = StdId;

	// Number of bytes sent (maximum 8 bytes)
	uint8_t maxLength = (DLC < 8) ? DLC : 8;
	txMessage.frame.dlc = maxLength;

	// Copy data from the TxData array to the fields of txMessage.frame.data:
	// txMessage.frame.data0; txMessage.frame.data1; txMessage.frame.data2; txMessage.frame.data3;
	// txMessage.frame.data4; txMessage.frame.data5; txMessage.frame.data6; txMessage.frame.data7;
	for (uint8_t i = 0; i < 8; i++) {
		// If index i is less than the actual length of the data, copy the data
		// Otherwise, assign 0 as the default value
		((uint8_t*)&txMessage.frame.data0)[i] = (i < maxLength) ? TxData[i] : 0;
	}

	// Try to add the message to the CAN bus
	if (CANSPI_Transmit(&txMessage)) {
		// Successfully queues the CAN message for transmission by finding an available buffer and requesting to send the data

		// Turn on the built-in LED to indicate successful transmission
		// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); // Green Led
	} else {
		// All transmission buffers are busy and thus no new message can be queued for transmission at that moment

		// Turn off the built-in LED to indicate an error during transmission
		// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		Error_Handler(); // Handle error if message transmission fails
	}
}

/**
 * ***********************************************************************
 * RECEIVE CAN MESSAGE
 * Receive a CAN message based on the specified parameters and selected
 * transceiver type. This function abstracts the reception process,
 * supporting different transceiver implementations.
 *
 * Use the appropriate reception function depending on the defined
 * transceiver macro (USE_SN65HVD230 or MCP2515).
 * ***********************************************************************
 */
void Receive_CAN_Message(CAN_HandleTypeDef *hcan)
{
	#ifdef USE_SN65HVD230
		if (hcan != NULL) Receive_CAN_Message_SN65HVD230(hcan);     // Use SN65HVD230 transceiver
    #else
    	Receive_CAN_Message_MCP2515();                		        // Use MCP2515 transceiver
    #endif
}

/* ***********************************************************************
 * SN65HVD230 MESSAGE RECEIVING
 * Handles the reception of CAN messages from the SN65HVD230 transceiver,
 * processes the messages based on their IDs, and updates corresponding
 * status flags.
 *
 * This function must be called from the pending CAN RX FIFO 0 message
 * interrupt service routine.
 *************************************************************************/
void Receive_CAN_Message_SN65HVD230(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[8]; // Buffer to store received CAN data

	// Check if there is a CAN message available on CAN2
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {
		// Turn on the built-in LED to indicate successful reception
		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); // Green Led

		// Process CAN messages based on their IDs
		switch (RxHeader.StdId) {
		case 0x33:
			// Extract data from CAN message
			accelData.x = (uint8_t) RxData[0];
			accelData.y = (uint8_t) RxData[1];
			accelData.z = (uint8_t) RxData[2];

			canStatusFlags.msgID_0x33 = 1;
			break;

		case 0x34:
			// Extract data from CAN message
			gyroData.x = (uint8_t) RxData[0];
			gyroData.y = (uint8_t) RxData[1];
			gyroData.z = (uint8_t) RxData[2];

			canStatusFlags.msgID_0x34 = 1;
			break;

		default:
			// Handle other CAN message IDs if needed
			break;
		}
	}else{
		// Turn off the built-in LED to indicate an error during reception
		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	}
}

/* ***********************************************************************
 * MCP2515 MESSAGE RECEIVING
 * Receives CAN messages from the MCP2515 transceiver, processes the
 * messages based on their IDs, and updates corresponding status flags.
 *
 * This function should be called regularly to ensure messages are
 * handled promptly and GPIO indicators are updated accordingly.
 *************************************************************************/
void Receive_CAN_Message_MCP2515() {
    /* Receive CAN message from MCP2515 transceiver */
    if (CANSPI_Receive(&rxMessage)) {
        // Process received message based on its ID
        switch (rxMessage.frame.id) {
            case 0x33:
                accelData.x = (uint8_t) rxMessage.frame.data0;     // Store accelerometer X data
                accelData.y = (uint8_t) rxMessage.frame.data1;     // Store accelerometer Y data
                accelData.z = (uint8_t) rxMessage.frame.data2;     // Store accelerometer Z data
                canStatusFlags.msgID_0x33 = 1;                     // Update status flag for message ID 0x33
                break;

            case 0x34:
                gyroData.x = (uint8_t) rxMessage.frame.data0;      // Store gyroscope X data
                gyroData.y = (uint8_t) rxMessage.frame.data1;      // Store gyroscope Y data
                gyroData.z = (uint8_t) rxMessage.frame.data2;      // Store gyroscope Z data
                canStatusFlags.msgID_0x34 = 1;                     // Update status flag for message ID 0x34
                break;
        }
        // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);        // Set GPIO pin high to indicate message received
    } else {
        // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);      // Reset GPIO pin if no message received
    }
}

/* ***********************************************************************
 * CAN MESSAGE DISPLAY
 * Displays received CAN messages via UART, formatting them for
 * transmission. The function checks the status flags for received
 * messages and sends the appropriate data.
 *
 * Messages are sent only once per reception to avoid duplication.
 *************************************************************************/
void Display_CAN_Messages() {
    /* Display received CAN messages via UART */
    if (canStatusFlags.msgID_0x33) {
        char accelMsg[50];                                          // Buffer for accelerometer message
        uint16_t accelMsgLength = sprintf(accelMsg, "Accel: x = %d; y = %d; z = %d\n", accelData.x, accelData.y, accelData.z);
        HAL_UART_Transmit(&huart2, (uint8_t*)accelMsg, accelMsgLength, HAL_MAX_DELAY); // Transmit accelerometer message
        canStatusFlags.msgID_0x33 = 0;                              // Reset status flag for message ID 0x33
    }

    if (canStatusFlags.msgID_0x34) {
        char gyroMsg[50];                                           // Buffer for gyroscope message
        uint16_t gyroMsgLength = sprintf(gyroMsg, "Gyro: x = %d; y = %d; z = %d\n", gyroData.x, gyroData.y, gyroData.z);
        HAL_UART_Transmit(&huart2, (uint8_t*)gyroMsg, gyroMsgLength, HAL_MAX_DELAY); // Transmit gyroscope message
        canStatusFlags.msgID_0x34 = 0;                              // Reset status flag for message ID 0x34
    }
}
