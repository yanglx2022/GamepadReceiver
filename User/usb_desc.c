/**********************
* USBÃèÊö·û
**********************/
#include "usb_desc.h"

// Éè±¸ÃèÊö·û
const uint8_t Gamepad_DeviceDescriptor[] =
{
	0x12,                       /*bLength */
	USB_DEVICE_DESCRIPTOR_TYPE, /*bDescriptorType*/
	WBVAL(0x0200),							/*bcdUSB 2.0*/
	0x00,                       /*bDeviceClass*/
	0x00,                       /*bDeviceSubClass*/
	0x00,                       /*bDeviceProtocol*/
	0x40,                       /*bMaxPacketSize 64*/
	WBVAL(0x0483),              /*idVendor (0x0483)*/
	WBVAL(0x0004),              /*idProduct = 0x0004*/
	WBVAL(0x0200),              /*bcdDevice rel. 2.00*/
	1,                          /*Index of string descriptor describing manufacturer */
	2,                          /*Index of string descriptor describing product*/
	3,                          /*Index of string descriptor describing the device serial number */
	0x01                        /*bNumConfigurations*/
};

// ÅäÖÃÃèÊö·û¼¯ºÏ
const uint8_t Gamepad_ConfigDescriptor[] =
{
	// ÅäÖÃÃèÊö·û
	0x09, 															/* bLength: Configuration Descriptor size */
	USB_CONFIGURATION_DESCRIPTOR_TYPE,	/* bDescriptorType: Configuration */
	WBVAL(GAMEPAD_SIZ_CONFIG_DESC),			/* wTotalLength: Bytes returned */
	0x01,         											/*bNumInterfaces: 1 interface*/
	0x01,         											/*bConfigurationValue: Configuration value*/
	0x00,         											/*iConfiguration: Index of string descriptor describing the configuration*/
	USB_CONFIG_BUS_POWERED,         		/*bmAttributes: bus powered */
	USB_CONFIG_POWER_MA(100),						/*MaxPower 100 mA: this current is used for detecting Vbus*/
	// ½Ó¿ÚÃèÊö·û
	0x09,         										/*bLength: Interface Descriptor size*/
	USB_INTERFACE_DESCRIPTOR_TYPE,		/*bDescriptorType: Interface descriptor type*/
	0x00,         										/*bInterfaceNumber: Number of Interface*/
	0x00,         										/*bAlternateSetting: Alternate setting*/
	0x01,         										/*bNumEndpoints*/
	USB_DEVICE_CLASS_HUMAN_INTERFACE,	/*bInterfaceClass: HID*/
	0x01,         										/*bInterfaceSubClass : 1=BOOT, 0=no boot*/
	0x02,         										/*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
	0x00,            									/*iInterface: Index of string descriptor*/
	// HIDÃèÊö·û
	0x09,         								/*bLength: HID Descriptor size*/
	HID_DESCRIPTOR_TYPE,					/*bDescriptorType: HID*/
	WBVAL(0x0100),								/*bcdHID: HID Class Spec release number 1.0*/
	0x00,         								/*bCountryCode: Hardware target country*/
	0x01,         								/*bNumDescriptors: Number of HID class descriptors to follow*/
	HID_REPORT_DESCRIPTOR_TYPE,		/*bDescriptorType*/
	WBVAL(GAMEPAD_SIZ_REPORT_DESC),	/*wItemLength: Total length of Report descriptor*/
	// ¶ËµãÃèÊö·û
	0x07,          								/*bLength: Endpoint Descriptor size*/
	USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/
	USB_ENDPOINT_IN(1),          	/*bEndpointAddress: Endpoint Address (IN)*/
	USB_ENDPOINT_TYPE_INTERRUPT,	/*bmAttributes: Interrupt endpoint*/
	WBVAL(0x0020),          			/*wMaxPacketSize: 32 Byte max */
	0x20,          								/*bInterval: Polling Interval (32 ms)*/
};

  ; /* MOUSE_ConfigDescriptor */
const uint8_t Gamepad_ReportDescriptor[] =
  {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    // USAGE (Game Pad)
    0xa1, 0x01,                    // COLLECTION (Application)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x08,                    //     USAGE_MAXIMUM (Button 8)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x08,                    //     REPORT_COUNT (8)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x32,                    //     USAGE (Z)
    0x09, 0x33,                    //     USAGE (Rx)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x04,                    //     REPORT_COUNT (4)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
  }
  ; /* Joystick_ReportDescriptor */

// ×Ö·û´®ÃèÊö·û
const uint8_t Gamepad_StringLangID[] =
  {
    GAMEPAD_SIZ_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09,
    0x04
  }
  ; /* LangID = 0x0409: U.S. English */
const uint8_t Gamepad_StringVendor[] =
  {
    GAMEPAD_SIZ_STRING_VENDOR, /* Size of Vendor string */
    USB_STRING_DESCRIPTOR_TYPE,  /* bDescriptorType*/
		'y', 0, 											/* yanglx2022 */
		'a', 0, 
		'n', 0, 
		'g', 0, 
		'l', 0, 
		'x', 0, 
		'2', 0, 
		'0', 0,
		'2', 0, 
		'2', 0
  };
const uint8_t Gamepad_StringProduct[] =
  {
    GAMEPAD_SIZ_STRING_PRODUCT,          /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
		'R', 0,												/* RC Gamepad */
		'C', 0,
		' ', 0,
		'G', 0,
		'a', 0,
		'm', 0,
		'e', 0,
		'p', 0,
		'a', 0,
		'd', 0
  };
uint8_t Gamepad_StringSerial[] =
  {
    GAMEPAD_SIZ_STRING_SERIAL,           /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0
  };

	
	

