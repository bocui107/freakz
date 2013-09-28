#ifndef raven_lcd_h
#define raven_lcd_h

PROCESS_NAME(raven_lcd_process);
int raven_lcd_serial_input(unsigned char ch);
void send_frame(uint8_t cmd, uint8_t len, uint8_t *payload);

/* Events for the Raven LCD processing loop */
#define SERIAL_CMD          0x1
#define SOF_CHAR            (0x01)  /**< Start-of-frame character. */
#define EOF_CHAR            (0x04)  /**< End-of-frame character. */
#define TERM_CHAR           (0x14)  /**< Control-T ASCII value for entering terminal mode. */

typedef enum
{
    CMD_TEMP               = 0x80,
    CMD_PING               = 0x81,
    CMD_START              = 0x82,
    CMD_JOIN               = 0x83,
    CMD_TX_DATA            = 0x84,
    CMD_SET_ADDR           = 0x85,

    REPORT_NWK_FORM_CONF    = 0xC0,
    REPORT_NWK_JOIN_CONF    = 0xC1,
    REPORT_TEXT_MSG         = 0xC2,
    REPORT_RX_DATA          = 0xC3
} uart_cmd_t;

typedef enum
{
    BUTTON_UP       = 0xA0,
    BUTTON_DOWN     = 0xA1,
    BUTTON_RIGHT    = 0xA2,
    BUTTON_LEFT     = 0xA3
} button_t;

#endif
