#!/bin/sh
#samples_size=10;
#samples=[-10, 20, -30, -40, -50, -120, -80, -60, -50, -20];
#value_array_size=128
#value=()
#echo -e "samples_size=[$value_array_size]\n" > ../samples.js
#echo -e "samples=[" >> ../samples.js
#for ((i=0; i < $value_array_size; i++)); do
#   	value[${i}]=${i}
#	echo -e "${value[$i]},\n" >> ../samples.js
#done
#echo -e "]" >> ../samples.js

echo -e "Content-type: text/html"
echo ""
echo -e "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \".w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
echo -e "<html xmlns=\".w3.org/1999/xhtml\" xmlns:mso=\"urn:schemas-microsoft-com:office:office\" xmlns:msdt=\"uuid:C2F41010-65B3-11d1-A29F-00AA00C14882\">"
echo -e "<head><title>Plotting Sample Values Example</title>"
echo -e "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">"
#echo -e "<link rel=\"stylesheet\" media=\"all\" type=\"text/css\" href=\"../style.css\" />"
#// JavaScript source code goes here
echo -e "<script src=\"MyGraphTest1_2.js\" type=\"text/javascript\"></script>"
echo -e "<script src=\"samples.js\" type=\"text/javascript\"></script>"
echo -e "</head>"
echo -e "<body onload=\"draw()\">"
#echo -e "<div class=\"bup-form\">"
echo -e "<div>"
echo -e "<span><strong><h1>Plotting Values from the Altera Cyclone-V SoC Development Kit (Using Web Server)</h1></strong><br/>"
echo -e "</span>"
echo -e "<p>This Example gets values from an FFT example design over HTTP to be displayed on the graph below."
echo -e "<p>"
echo -e "<p>"
echo -e "<p>"
#echo -e "<p> NUMBER OF SAMPLES DISPLAYED: $value_array_size"
echo -e "<p>"
echo -e "<p>"
echo -e "<p>"

echo -e "<FORM action=\"/fft.sh\" method=\"post\">"
echo -e "<P>"
#echo -e "<strong><font size=\"2\"> LED Lightshow: </font></strong> "
#echo -e "<INPUT type=\"text\" id=\"lightshow\" class=\"box\" size=\"22\" name=\"scroll_freq\" onChange=\"valuevalidation(this.value, 0);\" placeholder=\"Type LED Running Delay (ms)\">" 	
#echo -e "<INPUT type=\"submit\" class=\"box\" name=\"scroll\" value=\"START\" onclick=\"if(validatedelay()) return this.clicked  = true; else return this.clicked = false;\">"
echo -e "<INPUT type=\"submit\" value=\"RELOAD SAMPLE VALUES\" >"
echo -e "</P>"
echo -e "</FORM>"

echo -e "<p>"
echo -e "<p>"
echo -e "<p>"
echo -e "</div>"
echo -e "<canvas id=\"canvas\" width=\"1000\" height=\"250\"></canvas>"
echo -e "</body>"
echo -e "</html>"

