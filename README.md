# Robot detecting possible victims in a disaster environment

# Description
  Robot developed in partnership with UFU and the Civil Defense of Patos de Minas for the first disaster risk reduction workshop in Alto Paranaíba and Nororeste Mineiro.
  The project consists of a mobile robot capable of detecting the presence of people and sending their location in real time to a web server.
  
 # Materials Used and Development
   A robot with wheels capable of moving in the environment autonomously was used, for such functionality three ultrasonic sensors HC-SR04 were used in the prototype.
 The MLX90614 digital temperature sensor was used to detect the presence of people. The transmission of temperature and location information was done via LoRa
 using the LoRa / GPS Shield module and to receive the data a LoRa gateway LG01-P was used. The Things Network network server was used to receive this data
 and then they were sent to the Cayenne application server so that the user had access to a map to see the robot’s position and an icon on which
 issued an alert when the presence of a person was detected.
 
 # Authors
 
 Rafael Pereira - https://www.linkedin.com/in/rafael-pereira-73bba9148/
 Otávio Augusto  - https://github.com/OtaviodaCruz
 
   
