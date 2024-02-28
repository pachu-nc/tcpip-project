#Install following packages. To be done once in the host system
sudo apt-get install make cmake
sudo apt-get install git
sudo apt-get install doxygen
sudo apt-get install python3 python3-pip
sudo apt-get install graphviz


# Compile

make prepare;make

#Path for Library
# Library name libdll.a liblinked_list.a
build/lib

# Path of application . app name: tcpip_appl
build/app

#Path for Documentation
docs/html

