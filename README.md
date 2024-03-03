# Intern Projects

## Week 1
#### Projects done
- uart
- uart_ultra_sonic
- wifi_scan

## Week 2
This week, I mainly focused on very small project related to specific part of internet 
#### Projects done
- client 
  - Here, I used library from the example to connect to the internet as station.
  - then used the esp32 as a client to Get JSON data from specific site through https protocols with requried certification
  - also parsed the JSON data to get the required info
- server
  - after learning to use esp32 as client, I started to learn to use it as a server
  - this time, I wrote and used the required header files to use esp32 as station
  - then, I hosted a site with some html used
  - I also used MDNS to set a domain name(void-esp32.local)
  - Also I learnt to receive POST data (sent from the computer as client) 
  - I then like before parse the POSTED data(if valid)    
- websocket
  - finally as the final project of the week, i learnt to use the websocket protocol
  - this was somewhat similar to https except it was duplex and much faster
  - I also used this to receive JSON data and parse it like all the other projects from before    




