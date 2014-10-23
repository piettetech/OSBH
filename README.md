 <h1> Open Source Beehives Project Alpha Sensor Kit 1.0
  =====================================================</h1>

<pre>Developed for Open Source Beehives Project by Scott Piette [Piette Technologies, LTD]
Copyright 2014 Piette Technologies, LTD [piette.technologies@gmail.com]
Copyright 2014 Open Source Beehives Project http://www.opensourcebeehives.net
</pre>  
<h4>  <u>Spark Core Alpha 1.0 Version</u></h4>  
  <p>The Alpha sensor kit uses 2x DHT22 Temperature and Humidity sensors and 2x DS18B20 Temperature Sensors.  The application is built upon a unified sensor framework that allows for adding different sensors in the future.</p>
  <pre>  One of the DHT22 sensors is used for monitoring outdoor Temperature and Humidity
  One of the DHT22 sensors is used for monitoring hive Temperature and Humidity
  Two of the DS18B20 sensors are for monitoring Temperatures in other locations
  of the beehive.
  Data is collected every ten minutes and reported to a internet server running
  the SparkFun Phant database. </pre>
<h4><u>Prerequisites</u></h4>
  
  The installation instructions require that you have already installed 
  the Spark CLI tools.  See links below on how to install -
  
  [Windows]
  https://community.spark.io/t/tutorial-spark-cli-on-windows-01-oct-2014/3112
  
  [OSX]
  https://community.spark.io/t/tutorial-spark-cli-on-mac-osx-25-july-2014/5225
  
  Installation
  ------------
  Download the directory "OSBH" and unzip it into your working directory,
  then using the Spark CLI type the following :
  
  > spark compile OSBH --saveas osbh.bin
  
  This will upload all of the files to Spark, compile them, and download
  the a binary file for flashing.
  
  Next put your Spark into DFU mode by holding down the mode + reset, 
  release the reset button holding the mode button until the LED flashes
  yellow before releasing 
  the mode button
  
  > spark flash --usb osbh.bin
  
  This will flash the compiled program osbh.bin to your Spark connected to
  to the USB port.
  
  Hardware
  --------
  
</p>
