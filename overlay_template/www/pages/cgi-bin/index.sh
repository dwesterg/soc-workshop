#!/bin/sh
if [ "`cat /sys/devices/soc0/machine`" == "Altera SOCFPGA Cyclone V" ]; then
	MACHINE="cyclone5"
elif [ "`cat /sys/devices/soc0/machine`" == "Altera SOCFPGA Arria V" ]; then
	MACHINE="arria5"
else
	MACHINE="undef"
fi

case $MACHINE in
cyclone5)
	DEVKIT_NAME="Cyclone V SoC Development Kit"
	;;
arria5)
	DEVKIT_NAME="Arria V SoC Development Kit"
	;;
undef)
	DEVKIT_NAME="Unknown Development Kit"
	exit -1
	;;
esac

echo -e "Content-type: text/html"
echo ""
echo -e "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \".w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
echo -e "<html xmlns=\".w3.org/1999/xhtml\" xmlns:mso=\"urn:schemas-microsoft-com:office:office\" xmlns:msdt=\"uuid:C2F41010-65B3-11d1-A29F-00AA00C14882\">"
echo -e "<head>"
echo -e "<title>Board Update Portal: $DEVKIT_NAME</title>"
echo -e "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">"
echo -e "<link rel=\"stylesheet\" media=\"all\" type=\"text/css\" href=\"../style.css\" />"
echo -e "<script src=\"../AC_RunActiveContent.js\" type=\"text/javascript\"></script>"
echo -e "<script src=\"../progress.js\" type=\"text/javascript\"></script>"
echo -e "<script src=\"../helper_script.js\" type=\"text/javascript\"></script>"
echo -e "<script src=\"../validation_script.js\" type=\"text/javascript\"></script>"



echo -e "<!--[if gte mso 9]><xml>"
echo -e "<mso:CustomDocumentProperties>"
echo -e "<mso:ContentType msdt:dt=\"string\">Document</mso:ContentType>"
echo -e "</mso:CustomDocumentProperties>"
echo -e "</xml><![endif]-->"

echo -e "</head>"

echo -e "<body class=\"body-container\" onload=\"start()\">"
echo -e "<div class=\"bup-header\">"
echo -e "<img src=\"../altera-logo.gif\" alt=\"Altera Logo\" style=\"float:left; padding:15px 0px 15px 10px;\"/>"
echo -e "<div class=\"bup-header-right\"><span>Board Update Portal</span><br/>"
echo -e $DEVKIT_NAME
echo -e "</div>"
echo -e "</div>"
echo -e "<div class=\"bup-content\">"
echo -e "<div class=\"bup-form\">"
echo -e "<span><strong><h1>Overview</h1></strong><br/>"
echo -e "</span>"
echo -e "<p>This Board Update Portal web page is being served by the web server applicaiton running on the Hard Processor System (HPS) of your development board. This web page provides links to useful information on Altera<sup>®</sup> website. Please refer to the side bar for the reference links. You can use this web page to interact with your board by blinking the LEDs and writing text messages to LCD on the board. "
echo -e "</div>"


echo -e "<div class=\"bup-links\">"
echo -e "<h4>Developer Resources</h4>"
echo -e "<ul>"	
	echo -e "<li><a href=\"http://www.altera.com/soc-hw-resources.html\" target=\"_blank\">Hardware Developers</a></li> "
	echo -e "<li><a href=\"http://www.altera.com/soc-sw-resources.html\" target=\"_blank\">Software Developers</a></li>"
echo -e "</ul>"
echo -e "</div>"


echo -e "<div class=\"bup-form\">"
echo "<hr style=\"border: 1px solid; color:#06c\"><br>"
echo -e "<span><strong><h1>$DEVKIT_NAME Features</h1></strong><br/>"
echo -e "Mouse over the board photo to view features.</span>"

if [ "$MACHINE" == "cyclone5" ]; then
	echo -e "<span class=\"dev-kit-flash\">"
	echo -e "<script type=\"text/javascript\">"
	echo -e "AC_FL_RunContent( 'codebase','http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=9,0,28,0','width','640','height','478','src','cyclonev-board-flash','quality','high','pluginspage','http://www.adobe.com/shockwave/download/download.cgi?P1_Prod_Version=ShockwaveFlash','movie','../cyclonev-board-flash');"
	echo -e "</script>"
	echo -e "<noscript>"
	echo -e "<object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" codebase=\"http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=9,0,28,0\" width=\"640\" height=\"478\">"
	echo -e "<param name=\"movie\" value=\"cyclonev-board-flash.swf\" />"
	echo -e "<param name=\"quality\" value=\"high\" />"
	echo -e "<embed src=\"../cyclonev-board-flash.swf\" quality=\"high\" pluginspage=\"http://www.adobe.com/shockwave/download/download.cgi?P1_Prod_Version=ShockwaveFlash\" type=\"application/x-shockwave-flash\" width=\"640\" height=\"478\"></embed>"
	echo -e "</object>"
	echo -e "</noscript>"
	echo -e "</span>"
	echo -e "<div class=\"dev-kit-flash\"></div>"
	echo -e "</div>"
elif [ "$MACHINE" == "arria5" ]; then
	echo -e "<span class=\"dev-kit-flash\">"
	echo -e "<script type=\"text/javascript\">"
	echo -e "AC_FL_RunContent( 'codebase','http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=9,0,28,0','width','640','height','478','src','arriav-board-flash','quality','high','pluginspage','http://www.adobe.com/shockwave/download/download.cgi?P1_Prod_Version=ShockwaveFlash','movie','../arriav-board-flash');"
	echo -e "</script>"
	echo -e "<noscript>"
	echo -e "<object classid=\"clsid:D27CDB6E-AE6D-11cf-96B8-444553540000\" codebase=\"http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=9,0,28,0\" width=\"640\" height=\"478\">"
	echo -e "<param name=\"movie\" value=\"arriav-board-flash.swf\" />"
	echo -e "<param name=\"quality\" value=\"high\" />"
	echo -e "<embed src=\"../arriav-board-flash.swf\" quality=\"high\" pluginspage=\"http://www.adobe.com/shockwave/download/download.cgi?P1_Prod_Version=ShockwaveFlash\" type=\"application/x-shockwave-flash\" width=\"640\" height=\"478\"></embed>"
	echo -e "</object>"
	echo -e "</noscript>"
	echo -e "</span>"
	echo -e "<div class=\"dev-kit-flash\"></div>"
	echo -e "</div>"
fi

##
echo -e "<div id=\"interactive\" class=\"bup-form\">"
echo "<hr style=\"border: 1px solid; color:#06c\"><br>"
echo -e "<span><strong><h1>Interacting with $DEVKIT_NAME</h1></strong><br/>"
echo -e "</span>"

read POST_STRING
#echo $POST_STRING
SCROLL_DELAY=-1
LED_CONTROL=-1
LED_COMMAND="none"
LED_FREQ=-1

IFS='&' read -ra ADDR <<< "$POST_STRING"

for i in "${ADDR[@]}"
do
	KEY=`echo $i | sed 's/=.*//g'`
	VALUE=`echo $i | sed 's/.*=//g'`

	if [ "$KEY" = "lcd_text" ]; then
		printf '\e[2K' > /dev/ttyLCD0
		VALUE=`echo "$VALUE" | sed 's/+/ /g'`
		VALUE=`echo "$VALUE" | sed -e's/%\([0-9A-F][0-9A-F]\)/\\\\\x\1/g'`
		VALUE=`echo -e "$VALUE"`
		VALUE=`echo "$VALUE" | sed 's/%/%%/g' | sed 's/\\\\/\\\\\\\\/g'`
		printf "$VALUE" > /dev/ttyLCD0
	fi

	if [ "$KEY" = "scroll_freq" ]; then
		SCROLL_DELAY=$VALUE
	fi

	if [ "$KEY" = "scroll" ]; then
		if [ "$VALUE" = "START" ]; then
			SCROLL_COMMAND="start"
		fi
		if [ "$VALUE" = "STOP" ]; then
			SCROLL_COMMAND="stop"
		fi
	fi

	for LED_NUMBER in 0 1 2 3
	do
		if [ "$KEY" = "led_"$LED_NUMBER ]; then
			if [ "$VALUE" = "BLINK" ]; then
				LED_CONTROL=$LED_NUMBER
				LED_COMMAND="blink"
			fi
			if [ "$VALUE" = "OFF" ]; then
				LED_CONTROL=$LED_NUMBER
				LED_COMMAND="off"
			fi
			if [ "$VALUE" = "ON" ]; then
				LED_CONTROL=$LED_NUMBER
				LED_COMMAND="on"
			fi
		fi

	done

	if [ "$KEY" = "led_0_freq" ]; then
		LED_FREQ=$VALUE
	fi
	if [ "$KEY" = "led_1_freq" ]; then
		LED_FREQ=$VALUE
	fi
	if [ "$KEY" = "led_2_freq" ]; then
		LED_FREQ=$VALUE
	fi
	if [ "$KEY" = "led_3_freq" ]; then
		LED_FREQ=$VALUE
	fi
	if [ "$KEY" = "scroll_freq" ]; then
		SCROLL_FREQ=$VALUE
	fi
done

if [ "$LED_CONTROL" != "-1" ]; then
	if [ "$LED_COMMAND" = "blink" ]; then
		./blink $LED_CONTROL $LED_FREQ
	fi

	if [ "$LED_COMMAND" = "on" ]; then
		./toggle $LED_CONTROL 1
	fi

	if [ "$LED_COMMAND" = "off" ]; then
		./toggle $LED_CONTROL 0
	fi
fi

if [ "$SCROLL_COMMAND" = "start" ]; then
	./scroll_client $SCROLL_DELAY
fi
if [ "$SCROLL_COMMAND" = "stop" ]; then
	./scroll_client -1
fi

#ON=1, OFF=0, BLINK=-1
LED0_STATUS=1
LED1_STATUS=0
LED2_STATUS=1
LED3_STATUS=0
SCROLL_START=0

LED0_BLINKING=`cat /sys/class/leds/fpga_led0/trigger | cut -d "[" -f2 | cut -d "]" -f1`
LED1_BLINKING=`cat /sys/class/leds/fpga_led1/trigger | cut -d "[" -f2 | cut -d "]" -f1`
LED2_BLINKING=`cat /sys/class/leds/fpga_led2/trigger | cut -d "[" -f2 | cut -d "]" -f1`
LED3_BLINKING=`cat /sys/class/leds/fpga_led3/trigger | cut -d "[" -f2 | cut -d "]" -f1`

if [ "$LED0_BLINKING" = "timer" ]; then
LED0_STATUS=-1
else
LED0_STATUS=`cat /sys/class/leds/fpga_led0/brightness`
fi

if [ "$LED1_BLINKING" = "timer" ]; then
LED1_STATUS=-1
else
LED1_STATUS=`cat /sys/class/leds/fpga_led1/brightness`
fi

if [ "$LED2_BLINKING" = "timer" ]; then
LED2_STATUS=-1
else
LED2_STATUS=`cat /sys/class/leds/fpga_led2/brightness`
fi

if [ "$LED3_BLINKING" = "timer" ]; then
LED3_STATUS=-1
else
LED3_STATUS=`cat /sys/class/leds/fpga_led3/brightness`
fi

SCROLL_START=`./scroll_client 0`
if [ $SCROLL_START -ge 1 ]; then
SCROLL_START=1
else
SCROLL_START=0
fi

FPGA_IN_USER_MODE=`cat /sys/class/fpga/fpga0/status`
if [ "$FPGA_IN_USER_MODE" != "user mode" ]; then
echo -e "<p>FPGA is currently not programmed. LED control panel is disabled.</p>"
echo -e "<p>In order to display and control the LED via web or using command shell, "
echo -e "please program the FPGA before booting the system up. </p>"
else
echo -e "<p>You can observe the LED that are connected to the FPGA on the board from the picture below.</p>"

echo -e "<table style=\"margin-top:10px; margin-left:0px; font-family: Arial; font-size: 10pt\">"
echo -e "<tr><td></td><td align=center width=19 height=10>0</td> <td align=center width=19 height=10>1</td> <td align=center width=19 height=10>2</td> <td align=center width=19 height=10>3</td></tr>"
echo -e "<tr>"

if [ "$SCROLL_START" == "1" ]; then
echo -e "<td align=left ><strong>LED Status:</strong> </td> <td align=center colspan=4><img src=\"../runningled.gif\"></td>"
else
echo -e "<td><strong>LED Status:</strong></td>"
if [ "$LED0_STATUS" == "0" ]; then
	echo -e "<td align=center width=19 height=46> <img src=\"../offled.jpg\"> </td>"
elif [ "$LED0_STATUS" == "1" ]; then
	echo -e "<td align=center width=19 height=46> <img src=\"../onled.jpg\"> </td>"
else
	echo -e "<td align=center width=19 height=46> <img src=\"../blinkled.gif\"> </td>"
fi

if [ "$LED1_STATUS" == "0" ]; then
	echo -e "<td align=center width=19 height=46> <img src=\"../offled.jpg\"> </td>"
elif [ "$LED1_STATUS" == "1" ]; then
	echo -e "<td align=center width=19 height=46> <img src=\"../onled.jpg\"> </td>"
else
	echo -e "<td align=center width=19 height=46> <img src=\"../blinkled.gif\"> </td>"
fi

if [ "$LED2_STATUS" == "0" ]; then
	echo -e "<td align=center width=19 height=46> <img src=\"../offled.jpg\"> </td>"
elif [ "$LED2_STATUS" == "1" ]; then
	echo -e "<td align=center width=19 height=46> <img src=\"../onled.jpg\"> </td>"
else
	echo -e "<td align=center width=19 height=46> <img src=\"../blinkled.gif\"> </td>"
fi

if [ "$LED3_STATUS" == "0" ]; then
	echo -e "<td align=center width=19 height=46> <img src=\"../offled.jpg\"> </td>"
elif [ "$LED3_STATUS" == "1" ]; then
	echo -e "<td align=center width=19 height=46> <img src=\"../onled.jpg\"> </td>"
else
	echo -e "<td align=center width=19 height=46> <img src=\"../blinkled.gif\"> </td>"
fi
fi

echo -e "</tr>"
echo -e "</table>"

echo "<br><hr style=\"border: 1px dotted\"><br>"


echo -e "<p>You can start running lightshow on the LED that are connected to the FPGA. Type in the running delay in milliseconds and click Start button. Click Stop button when you wish to stop the running LED.<br><br></p>"

echo -e "<FORM name=\"interactive\" action=\"/cgi-bin/index.sh#interactive\" method=\"post\">"


	echo -e "<P>"
	echo -e "<strong><font size=\"2\"> LED Lightshow: </font></strong> "
	echo -e "<INPUT type=\"text\" id=\"lightshow\" class=\"box\" size=\"22\" name=\"scroll_freq\" onChange=\"valuevalidation(this.value, 0);\" placeholder=\"Type LED Running Delay (ms)\">"
    echo -e "<INPUT type=\"submit\" class=\"box\" name=\"scroll\" value=\"START\" onclick=\"if(validatedelay()) return this.clicked  = true; else return this.clicked = false;\">"
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"scroll\" value=\"STOP\" >"
	echo -e "</P>"

echo -e "</FORM>"

echo "<br>"

echo "<hr style=\"border: 1px dotted\">"

echo -e "<p><br>You can control to turn on, turn off or to blink the LED that are connected to the FPGA on the development kit. To blink the LED, type the LED toggling delay in milliseconds and click Blink button.<br><br></p>"

echo -e "<FORM action=\"/cgi-bin/index.sh#interactive\" method=\"post\">"
    echo -e "<P>"
    echo -e "<strong><font size=\"2\"> LED 0: </font></strong> "	
	if [ "$SCROLL_START" == 0 ]; then
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_0\" value=\"ON\" >"
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_0\" value=\"OFF\" >"
	echo -e "&nbsp &nbsp &nbsp &nbsp &nbsp"
	echo -e "<INPUT type=\"text\" id=\"led0_id\" class=\"box\" size=\"22\" name=\"led_0_freq\" placeholder=\"Type LED Toggling Delay (ms)\" onChange=\"valuevalidation(this.value, 1);\">  " 	
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_0\" value=\"BLINK\" >"
	else
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_0\" value=\"ON\" disabled>"
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_0\" value=\"OFF\" disabled>"
	echo -e "&nbsp &nbsp &nbsp &nbsp &nbsp"
	echo -e "<INPUT type=\"text\" id=\"led0_id\" class=\"box\" size=\"22\" name=\"led_0_freq\" placeholder=\"Type LED Toggling Delay (ms)\"  disabled>  "
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_0\" value=\"BLINK\" disabled>"
	fi	
    echo -e "</P>"
echo -e "</FORM>"

echo -e "<FORM action=\"/cgi-bin/index.sh#interactive\" method=\"post\">"
	echo -e "<P>"
    echo -e "<strong><font size=\"2\"> LED 1: </font></strong> "	
	if [ "$SCROLL_START" == 0 ]; then
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_1\" value=\"ON\" >"
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_1\" value=\"OFF\" >"	
	echo -e "&nbsp &nbsp &nbsp &nbsp &nbsp"
	echo -e "<INPUT type=\"text\" id=\"led1_id\" class=\"box\" size=\"22\" name=\"led_1_freq\" placeholder=\"Type LED Toggling Delay (ms)\" onChange=\"valuevalidation(this.value, 2);\">  "
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_1\" value=\"BLINK\" >"
	else
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_1\" value=\"ON\" disabled>"
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_1\" value=\"OFF\" disabled>"	
	echo -e "&nbsp &nbsp &nbsp &nbsp &nbsp"
	echo -e "<INPUT type=\"text\" id=\"led1_id\" class=\"box\" size=\"22\" name=\"led_1_freq\" placeholder=\"Type LED Toggling Delay (ms)\" disabled>  "
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_1\" value=\"BLINK\" disabled>"
	fi	
    echo -e "</P>"
echo -e "</FORM>"

echo -e "<FORM action=\"/cgi-bin/index.sh#interactive\" method=\"post\">"
	echo -e "<P>"
    echo -e "<strong><font size=\"2\"> LED 2: </font></strong> "	
	if [ "$SCROLL_START" == 0 ]; then
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_2\" value=\"ON\" >"
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_2\" value=\"OFF\" >"
	echo -e "&nbsp &nbsp &nbsp &nbsp &nbsp"
	echo -e "<INPUT type=\"text\" id=\"led2_id\" class=\"box\" size=\"22\" name=\"led_2_freq\" placeholder=\"Type LED Toggling Delay (ms)\"  onChange=\"valuevalidation(this.value, 3);\">  "
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_2\" value=\"BLINK\" >"
	else
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_2\" value=\"ON\" disabled>"
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_2\" value=\"OFF\" disabled>"	
	echo -e "&nbsp &nbsp &nbsp &nbsp &nbsp"
	echo -e "<INPUT type=\"text\" id=\"led2_id\" class=\"box\" size=\"22\" name=\"led_2_freq\" placeholder=\"Type LED Toggling Delay (ms)\" disabled>  "
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_2\" value=\"BLINK\" disabled>"
	fi
	echo -e "</P>"
echo -e "</FORM>"

echo -e "<FORM action=\"/cgi-bin/index.sh#interactive\" method=\"post\">"
	echo -e "<P>"
    echo -e "<strong><font size=\"2\"> LED 3: </font></strong> "	
	if [ "$SCROLL_START" == 0 ]; then
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_3\" value=\"ON\" >"
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_3\" value=\"OFF\" >"	
	echo -e "&nbsp &nbsp &nbsp &nbsp &nbsp"
	echo -e "<INPUT type=\"text\" id=\"led3_id\" class=\"box\" size=\"22\" name=\"led_3_freq\" placeholder=\"Type LED Toggling Delay (ms)\"  onChange=\"valuevalidation(this.value, 4);\">  "
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_3\" value=\"BLINK\" >"
	else
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_3\" value=\"ON\" disabled>"
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_3\" value=\"OFF\" disabled>"	
	echo -e "&nbsp &nbsp &nbsp &nbsp &nbsp"
	echo -e "<INPUT type=\"text\" id=\"led3_id\" class=\"box\" size=\"22\" name=\"led_3_freq\" placeholder=\"Type LED Toggling Delay (ms)\" disabled>  "
	echo -e "<INPUT type=\"submit\" class=\"box\" name=\"led_3\" value=\"BLINK\" disabled>"
	fi
	echo -e "</P>"	
echo -e "</FORM>"

# FPGA in user mode detection complete
fi


 echo "<br><hr id=\"lcd\" style=\"border: 1px dotted\">"

 echo -e "<p><br>Type in the message (maximum 16 characters) that you wish to send over to the character LCD on the development kit. <br><br></p>"

 echo -e "<FORM action=\"/cgi-bin/index.sh#lcd\" method=\"post\">"


	echo -e "<P>"
    echo -e "<strong><font size=\"2\">Send to character LCD: </font></strong> "
	echo -e "<INPUT type=\"text\" class=\"box\" name=\"lcd_text\" maxlength=\"16\">"
    echo -e "<INPUT type=\"submit\" class=\"box\" value=\"Send to LCD\"> "
    echo -e "</P>"

 echo -e "</form>"


 echo -e "<br> <hr style=\"border: 1px solid; color:#06c\"> <br>"
 echo -e "<span><strong><h1>Connect to Linux Console on board over SSH Connection</h1></strong><br>"
 echo -e "<p>You may connect your host system to SSH server running on the board using the IP address displayed on the character LCD. In your host system terminal, type the following:<br><br> <font face="courier, arial" size="3">ssh root@[IP address]</font></p>"
 echo -e "<p>You must have SSH client installed in your host system. If SSH is not available, install the openssh package.</p>"
 echo -e "<p>When the SSH connection is established, you may find the Readme material in /home/root to obtain instructions on how to run SoC Linux example applications.</p>"


echo -e "</div>"

echo -e "</div>"

echo -e "<div class=\"footer-gutter\"></div>"

echo -e "<div class=\"footer-container-blur\">"
echo -e "<div class=\"footer-container-shadow\">"
echo -e "<div id=\"footer\" class=\"footer-container\">"

	echo -e "<div id=\"footerCopyright\" class=\"footer-copyright\">"
	echo -e "<p align=\"center\" style=\"width:900px; font-size:9px;\">Copyright &copy; 1995-2013 Altera Corporation. All Rights Reserved.<br/>"
	echo -e "ALTERA, ARRIA, CYCLONE, HARDCOPY, MAX, MEGACORE, NIOS, QUARTUS &amp; STRATIX are Reg. U.S. Pat. &amp; Tm. Off. and Altera marks in and outside the U.S.</p></div>"
echo -e "</div>"
echo -e "</div>"
echo -e "</div>"

echo -e "</body>"
echo -e "</html>"

