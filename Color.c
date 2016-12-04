//Sta da radimo sa ovim??
//#include <built_in.h>
#include "i2c.h"
#include "stm32f10x.h"

// TCS3471 registers definition
#define _ENABLE  0x80    // Enable status and interrupts
#define _ATIME   0x81    // RGBC ADC time
#define _WTIME   0x83    // Wait time
#define _AILTL   0x84    // RGBC Interrupt low treshold low byte
#define _AILTH   0x85    // RGBC Interrupt low treshold high byte
#define _AIHTL   0x86    // RGBC Interrupt high treshold low byte
#define _AIHTH   0x87    // RGBC Interrupt high treshold high byte
#define _PERS    0x8C    // Interrupt persistence filters
#define _CONFIG  0x8D    // Configuration
#define _CONTROL 0x8F    // Gain control register
#define _ID      0x92    // Device ID
#define _STATUS  0x93    // Device status
#define _CDATA   0x94    // Clear ADC low data register
#define _CDATAH  0x95    // Clear ADC high data register
#define _RDATA   0x96    // RED ADC low data register
#define _RDATAH  0x97    // RED ADC high data register
#define _GDATA   0x98    // GREEN ADC low data register
#define _GDATAH  0x99    // GREEN ADC high data register
#define _BDATA   0x9A    // BLUE ADC low data register
#define _BDATAH  0x9B    // BLUE ADC high data register
#define _COLOR_W_ADDRESS 0x52
#define _COLOR_R_ADDRESS 0x53

// Color flags
#define PURPLE_FLAG 1
#define BLUE_FLAG   2
#define CYAN_FLAG   3
#define GREEN_FLAG  4
#define PINK_FLAG   5
#define RED_FLAG    6
#define ORANGE_FLAG 7
#define YELLOW_FLAG 8

// Gain constants
#define _GAIN_x4 1  0x01
#define _GAIN_x16   0x10
#define _GAIN_x60   0x11

// Find maximal floating point value
#define MAX_FLOAT(a, b) (((a) > (b)) ? (a) : (b))

// Find minimal floating point value
#define MIN_FLOAT(a, b) (((a) < (b)) ? (a) : (b))


// Variable declaration
char i, color_detected, color_flag;
unsigned int Clear, Red, Green, Blue;
float hue, color_value, color_value_sum;
float Red_Ratio, Green_Ratio, Blue_Ratio;




// Write value into TCS3471 Color sensor
void Color_Write(unsigned short address, unsigned short data1) {
	
	I2C_Write(I2C1,&data1,1,address,_COLOR_W_ADDRESS); 
	
	/*	
	  I2C2_Start();                       // issue I2C start signal
	  I2C2_Write(_COLOR_W_ADDRESS);       // send byte via I2C  (device address + W)
	  I2C2_Write(address);                // send byte (address of the location)
	  I2C2_Write(data1);                  // send data (data to be written)
	  I2C2_Stop();                        // issue I2C stop signal
	*/
}

// Read value from the TCS3471 Color sensor
unsigned short Color_Read(unsigned short address) {
  unsigned short tmp = 0;
  
  I2C_Read(I2C1,&tmp,1,address,_COLOR_R_ADDRESS);
  
  /*   
  I2C2_Start();                       // issue I2C start signal
  I2C2_Write(_COLOR_W_ADDRESS);       // send byte via I2C (device address + W)
  I2C2_Write(address);                // send byte (data address)

  I2C2_Restart();                     // issue I2C signal repeated start
  I2C2_Write(_COLOR_R_ADDRESS);       // send byte (device address + R)
  tmp = I2C2_Read(_I2C_NACK);         // Read the data (NO acknowledge)
  I2C2_Stop();                        // issue I2C stop signal
  */
  return tmp;
}

// Initialize TCS3471 Color sensor
void Color_Init() {
  Color_Write(_ENABLE, 0x1B);
}

// Read RGB values from the TCS3471 Color sensor
unsigned int Color_Read_value(char reg) {
  unsigned short low_byte;
  unsigned int Out_color;

  switch(reg) {
    case 'C': low_byte = Color_Read(_CDATA);
              Out_color = Color_Read(_CDATAH);
              Out_color = (Out_color << 8);
              Out_color = (Out_color | low_byte);
              return Out_color;
              break;

    case 'R': low_byte = Color_Read(_RDATA);
              Out_color = Color_Read(_RDATAH);
              Out_color = (Out_color << 8);
              Out_color = (Out_color | low_byte);
              return Out_color;
              break;

    case 'G': low_byte = Color_Read(_GDATA);
              Out_color = Color_Read(_GDATAH);
              Out_color = (Out_color << 8);
              Out_color = (Out_color | low_byte);
              return Out_color;
              break;

    case 'B': low_byte = Color_Read(_BDATA);
              Out_color = Color_Read(_BDATAH);
              Out_color = (Out_color << 8);
              Out_color = (Out_color | low_byte);
              return Out_color;
              break;

    default:  return 0;
  }
}

/*
// Configure RGB LEDs
void Configure_RGB_LEDs() {
  // Set RGB LEDs as output
  RED_LED_Direction = 0;
  GREEN_LED_Direction = 0;
  BLUE_LED_Direction = 0;

  // Clear RGB LEDs PORT bits
  RB8_bit = 0;
  RC2_bit = 0;
  RD0_bit = 0;
}

// Turn on RGB LEDs
void Turn_ON_RGB_LEDs() {
  RED_LED = 1;
  GREEN_LED = 1;
  BLUE_LED = 1;
}

// Turn off RGB LEDs
void Turn_OFF_RGB_LEDs() {
  RED_LED = 0;
  GREEN_LED = 0;
  BLUE_LED = 0;
}
*/


// Convert RGB values to HSL values
float RGB_To_HSL(float red, float green, float blue) {
  float volatile fmax, fmin, hue, saturation, luminance;
 
  fmax = MAX_FLOAT(MAX_FLOAT(red, green), blue);
  fmin = MIN_FLOAT(MIN_FLOAT(red, green), blue);
 
  luminance = fmax;
  if (fmax > 0)
    saturation = (fmax - fmin) / fmax;
  else
    saturation = 0;
 
  if (saturation == 0)
    hue = 0;
  else {
    if (fmax == red)
      hue = (green - blue) / (fmax - fmin);
    else if (fmax == green)
      hue = 2 + (blue - red) / (fmax - fmin);
    else
      hue = 4 + (red - green) / (fmax - fmin);
    hue = hue / 6;
  
    if (hue < 0)
      hue += 1;
  }
  return hue;
}

// UART write text and new line (carriage return + line feed)
void UART_Write_Line(char *uart_text) {
  /*
     Zameniti sa STM-ovim funkcijama
  */
  /*
  UART2_Write_Text(uart_text);
  UART2_Write(13);
  UART2_Write(10);
  */
}

void main() {
  /*   
     Dodati inicijalizaciju za STM module
  */  
  
  
  // Initialize pins as digital I/O
  //AD1PCFG = 0xFFFF;
  //JTAGEN_bit = 0;
  
  // Initialize variables
  color_value_sum = 0;
  color_detected = 0;

  /*
  // Configure RGB LEDs
  Configure_RGB_LEDs();

  // Turn OFF RGB LEDs
  Turn_OFF_RGB_LEDs();
  */
  

  // Initialize I2C2 module
  I2C_LowLevel_Init(I2C1,400000, OwnAddress1);
  //I2C2_Init(400000);
  Delay_ms(100);
  
  // Initialize TCS3471 Color Sensor
  Color_Init();
  Delay_ms(100);

  // Set gain x16
  Color_Write(_CONTROL, _GAIN_x16);

  // Set acquisition timer of 700 ms
  Color_Write(_ATIME, 0x00);

  // Initialize UART2 module
  UART2_Init(19200);
  Delay_ms(100);

  // Display message
  UART_Write_Line("Color Click Test Example");
	
  /*
  // Turn OFF RGB LEDs
  Turn_ON_RGB_LEDs();
  */
  
  while(1) {
    // Reset Color value sum variable
    color_value_sum = 0;
    
    // Get the average color value of 16 measurements
    for(i = 0; i<16; i++) {
      // Read Clear, Red, Green and Blue channel register values
      Clear = Color_Read_value('C');
      Red   = Color_Read_value('R');
      Green = Color_Read_value('G');
      Blue  = Color_Read_value('B');

      // Divide Red, Green and Blue values with Clear value
      Red_Ratio   = ((float)Red   / (float)Clear);
      Green_Ratio = ((float)Green / (float)Clear);
      Blue_Ratio  = ((float)Blue  / (float)Clear);

      // Convert RGB values to HSL values
      color_value = RGB_To_HSL(Red_Ratio, Green_Ratio, Blue_Ratio);
      
      // Sum the color values
      color_value_sum = color_value_sum + color_value;
    }

    // Get the average color value of 16 measurements
    color_value = color_value_sum / 16.0;

    // Orange color range
    if ((color_value >= 0.992) && (color_value <= 0.999)) {
      color_detected = 1;
      if (color_flag != ORANGE_FLAG){
        color_flag = ORANGE_FLAG;
        UART_Write_Line("Orange");
      }
    }

    // Red color range
    else if ((color_value >= 0.9750) && (color_value <= 0.9919)) {
      color_detected = 1;
      if (color_flag != RED_FLAG){
        color_flag = RED_FLAG;
        UART_Write_Line("Red");
      }
    }

    // Pink color range
    else if ((color_value >= 0.920) && (color_value <= 0.9749)) {
      color_detected = 1;
      if (color_flag != PINK_FLAG){
        color_flag = PINK_FLAG;
        UART_Write_Line("Pink");
      }
    }

    // Purple color range
    else if ((color_value >= 0.6201) && (color_value <= 0.919)) {
      color_detected = 1;
      if (color_flag != PURPLE_FLAG){
        color_flag = PURPLE_FLAG;
        UART_Write_Line("Purple");
      }
    }

    // Blue color range
    else if ((color_value >= 0.521) && (color_value <= 0.6200)) {
      color_detected = 1;
      if (color_flag != BLUE_FLAG){
        color_flag = BLUE_FLAG;
        UART_Write_Line("Blue");
      }
    }

    // Cyan color range
    else if ((color_value >= 0.470) && (color_value < 0.520)) {
      color_detected = 1;
      if (color_flag != CYAN_FLAG){
        color_flag = CYAN_FLAG;
        UART_Write_Line("Cyan");
      }
    }

    // Green color range
    else if ((color_value >= 0.210) && (color_value <= 0.469)) {
      color_detected = 1;
      if (color_flag != GREEN_FLAG){
        color_flag = GREEN_FLAG;
        UART_Write_Line("Green");
      }
    }

    // Yellow color range
    else if ((color_value >= 0.0650) && (color_value <= 0.1800)) {
      color_detected = 1;
      if (color_flag != YELLOW_FLAG){
        color_flag = YELLOW_FLAG;
        UART_Write_Line("Yellow");
      }
    }

    // Color not in range
    else {
      if (color_detected == 0){
        color_flag = 0;
        Delay_ms(500);
        UART_Write_Line("Color not in range.");
      }
      else {
        color_detected = 0;
      }
    }
  }
}
