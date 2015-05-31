#!/usr/bin/env python
# -*- coding: utf-8 -*-
import threading
from ComssServiceDevelopment.connectors.tcp.msg_stream_connector import InputMessageConnector, OutputMessageConnector #import modułów konektora msg_stream_connector
from ComssServiceDevelopment.connectors.tcp.object_connector import InputObjectConnector, OutputObjectConnector #import modułów konektora object_connector
from ComssServiceDevelopment.service import Service, ServiceController #import modułów klasy bazowej Service oraz kontrolera usługi
import cv2 #import modułu biblioteki OpenCV
import numpy as np #import modułu biblioteki Numpy
import json

class Filter1Service(Service):
    def __init__(self):
        super(Filter1Service, self).__init__()
        self.filters = []
        self.filters_lock = threading.RLock()

    def declare_outputs(self): #deklaracja wyjść
        self.declare_output("videoOutput", OutputMessageConnector(self))
        self.declare_output("debugOutput", OutputObjectConnector(self))

    def declare_inputs(self):
        self.declare_input("videoInput", InputMessageConnector(self))
        self.declare_input("objectsInput", InputObjectConnector(self))

    def run(self):

        video_input = self.get_input("videoInput")
        video_output = self.get_output("videoOutput")
        objects_input = self.get_input("objectsInput")
        debug_output = self.get_output("debugOutput")
        
        with open('config.json') as data_file:
            data = json.load(data_file)

        classes = data["conf"]["classes"]
        
        frame = None

        while self.running(): #pętla główna usługi (wątku głównego obsługującego strumień wideo)
            
            frame_obj = video_input.read() #odebranie danych z interfejsu wejściowego
            frame = np.loads(frame_obj) #załadowanie ramki do obiektu NumPy

            objects = objects_input.read()

            size_objects = len(objects)

            for i in range(0,size_objects):
                o = objects[i]
                object_class = "class" + str(o[3])
                color = classes[object_class]
                cv2.rectangle(frame, (o[0], o[1]), (o[0]+o[2], o[1]+o[2]), (color[0],color[1],color[2]), 3)
            #cv2.rectangle(frame, (20, 20), (100, 100), (0,255,0), 3)
            
            video_output.send(frame.dumps())
            debug_output.send([])

if __name__=="__main__":
    sc = ServiceController(Filter1Service, "picasso.json") #utworzenie obiektu kontrolera usługi
    sc.start() #uruchomienie usługi
