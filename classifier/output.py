#!/usr/bin/env python
# -*- coding: utf-8 -*-
import Tkinter as tk
import threading

from ComssServiceDevelopment.connectors.tcp.msg_stream_connector import InputMessageConnector #import modułów konektora msg_stream_connector
from ComssServiceDevelopment.connectors.tcp.object_connector import InputObjectConnector
from ComssServiceDevelopment.development import DevServiceController #import modułu klasy testowego kontrolera usługi
import cv2 #import modułu biblioteki OpenCV
import numpy as np #import modułu biblioteki Numpy

service_controller = DevServiceController("picasso.json") #utworzenie obiektu kontroletra testowego, jako parametr podany jest plik konfiguracji usługi, do której "zaślepka" jest dołączana

service_controller.declare_connection("videoOutput", InputMessageConnector(service_controller)) 
service_controller.declare_connection("debugOutput", InputObjectConnector(service_controller))

connection = service_controller.get_connection("videoOutput")
conn_objs = service_controller.get_connection("debugOutput")
while True: #główna pętla programu
    
    objects = conn_objs.read()
    
    obj = connection.read() #odczyt danych z interfejsu wejściowego
    frame = np.loads(obj) #załadownaie ramki do obiektu NumPy
    
    #print objects
            
    cv2.imshow('Preview',frame) #wyświetlenie ramki na ekran
    cv2.waitKey(1)
