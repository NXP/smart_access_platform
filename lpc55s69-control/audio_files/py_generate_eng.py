import sys
import os
import comtypes.client

import string, time, re, math, fileinput, glob, shutil, stat
import serial, time

def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

# main process
def main(object):

    print("Making an audio binary")
    # Get Project Root Path
    prjrootpath = os.path.abspath('..')
    print("Project Root Path is " + prjrootpath)
            
    prjuserpath  = prjrootpath + "\\audio_files"
    os.chdir(prjuserpath)

    print("Creating Audio Binary")
    os.remove("audio_bin.bin")

    bin_object = open("audio_bin.bin", 'wb+')

    audio_file = ['0-eng.mp3','1-eng.mp3','2-eng.mp3','3-eng.mp3','4-eng.mp3','5-eng.mp3','6-eng.mp3','7-eng.mp3','8-eng.mp3','9-eng.mp3','10-eng.mp3','11-eng.mp3','12-eng.mp3','13-eng.mp3','14-eng.mp3','15-eng.mp3','16-eng.mp3', '17-eng.mp3', '18-eng.mp3', '19-eng.mp3', '20-eng.mp3', '21-eng.mp3', '22-eng.mp3', '23-eng.mp3', '24-eng.mp3', '25-eng.mp3', '26-eng.mp3', '27-eng.mp3', '28-eng.mp3', '29-eng.mp3', '30-eng.mp3', '31-eng.mp3', '32-eng.mp3', '33-eng.mp3']
    
    file_num = 0

    for file_num in range(len(audio_file)):
        file_object = open(audio_file[file_num], 'rb')
        CHS_X_BIN = file_object.read()
        file_object.close()
        bin_object.seek(16384*file_num, 0)
        bin_object.write(CHS_X_BIN)
        file_object.close()
        print("Copy  "+audio_file[file_num]+"  into audio_bin.bin end")

    print("Convert Audio Image Done")

    print(serial_ports())

    ser = serial.Serial()
    #ser.port = "/dev/ttyUSB0"
    ser.port = "COM34"
    #ser.port = "/dev/ttyS2"
    ser.baudrate = 115200
    ser.bytesize = serial.EIGHTBITS #number of bits per bytes
    ser.parity = serial.PARITY_NONE #set parity check: no parity
    ser.stopbits = serial.STOPBITS_ONE #number of stop bits
    #ser.timeout = None          #block read
    ser.timeout = 1            #non-block read
    #ser.timeout = 2              #timeout block read
    ser.xonxoff = False     #disable software flow control
    ser.rtscts = False     #disable hardware (RTS/CTS) flow control
    ser.dsrdtr = False       #disable hardware (DSR/DTR) flow control
    ser.writeTimeout = 2     #timeout for write
    
    try: 
        ser.open()
    except Exception as e:
        print ("error open serial port: %s" %str(e) )
        exit()

    bin_object = open("audio_bin.bin", 'rb')

    ack_status = 0

    if ser.isOpen():
        try:
            ser.flushInput() #flush input buffer, discarding all its contents
            ser.flushOutput()#flush output buffer, aborting current output 
                     #and discard all that is in buffer
            ser.write(str.encode("UPDATEBIN\r\n"))

            numOfLines = 0

            while True:
                time.sleep(1)
                response = ser.readline()
                print("read data: %s" %response)
                if(response == b'OK'):
                    ack_status = 1
                    print("Received OK")
                    break
                if (numOfLines >= 5):
                    break

            if (ack_status == 1):
                print("Update audio binary started")
                for read_num in range(0, 34):
                    send_buf = bin_object.read(16384)
                    ser.write(send_buf)
                    print("write data %s" %chr(read_num) )

                    numOfLines = 0

                    while True:
                        time.sleep(1)
                        response = ser.readline()
                        print("read data: %s" %response)
                        if(response == "TimeOut"):
                            numOfLines = 5
                            break

                        numOfLines = numOfLines + 1
                        if (numOfLines >= 5):
                            break


            ser.close()

        except Exception as e1:
            print ("error communicating...: %s" %str(e1) )

    else:
        print ("cannot open serial port ")

    bin_object.close()

if __name__ == '__main__':
    # argv[1] = IDE, argv[2] = project name
    main(sys.argv)
#    if not loghandel.closed:
#        loghandel.flush()
#        closelog()
