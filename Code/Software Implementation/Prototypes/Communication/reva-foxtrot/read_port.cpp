#include <SerialStream.h>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
typedef  unsigned char  uchar;
using namespace std;
using namespace LibSerial ;
SerialStream serial_port ;

void SendHeader(uchar groupid , uchar cmd, uchar size  )
{
    uchar checksum  = groupid+ cmd +size;


    serial_port.write( ( char *)&groupid, 1 ) ;
    serial_port.write( ( char *)&cmd, 1 ) ;
    serial_port.write( ( char *)&size, 1 ) ;
    serial_port.write( ( char *)&checksum, 1 ) ;




}


main(  )
{
    //
    // Open the serial port.
    //
    serial_port.Open( "/dev/ttyUSB0" ) ;
    if ( ! serial_port.good() ) 
    {
        std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] "
                  << "Error: Could not open serial port." 
                  << std::endl ;
        exit(1) ;
    }
    //
    // Set the baud rate of the serial port.
    //
    serial_port.SetBaudRate( SerialStreamBuf::BAUD_9600 ) ;
    if ( ! serial_port.good() )
    {
        std::cerr << "Error: Could not set the baud rate." << std::endl ;
        exit(1) ;
    }
    //
    // Set the number of data bits.
    //
    serial_port.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 ) ;
    if ( ! serial_port.good() ) 
    {
        std::cerr << "Error: Could not set the character size." << std::endl ;
    }
    //
    // Disable parity.
    //
    serial_port.SetParity( SerialStreamBuf::PARITY_NONE ) ;
    if ( ! serial_port.good() ) 
    {
        std::cerr << "Error: Could not disable the parity." << std::endl ;
        exit(1) ;
    }
    //
    // Set the number of stop bits.
    //
    serial_port.SetNumOfStopBits( 1 ) ;
    if ( ! serial_port.good() ) 
    {
        std::cerr << "Error: Could not set the number of stop bits."
                  << std::endl ;
        exit(1) ;
    }
    //
    // Turn off hardware flow control.
    //
    serial_port.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_NONE ) ;
    if ( ! serial_port.good() ) 
    {
        std::cerr << "Error: Could not use hardware flow control."
                  << std::endl ;
        exit(1) ;
    }
    //
    // Do not skip whitespace characters while reading from the
    // serial port.
    //
    // serial_port.unsetf( std::ios_base::skipws ) ;
    //
    // Wait for some data to be available at the serial port.
    //




        usleep(100) ;
        int size = 3;
        SendHeader(1,1,size);
        char next_byte;
         serial_port.get(next_byte);
         if((uchar)next_byte == 255)
             std::cout << "good shit\n";
         else
             std:cout << "bad shit\n";

        unsigned char array[10] = {0xff,2,3,6, 4,4,4};
        uchar checksum = 0;
        for(int i = 0;  i < size; i++)
        {
            serial_port.write( &(((const char *)array)[i]), 1 ) ;
            checksum  += array[i];


        cout << "sent "<< "\n";

        }
         serial_port.write((char *)&checksum,1);

         serial_port.get(next_byte);
         if((uchar)next_byte == 255)
             std::cout << "good shit\n";
         else
             std::cout << "bad shit\n";


        //std::cout << "recieved bytes:" <<next_byte << "\n";

 //       std::cerr << std::hex << static_cast<int>(next_byte) << " ";
        usleep(100) ;


   std::cerr << std::endl ;
    return EXIT_SUCCESS ;
}
