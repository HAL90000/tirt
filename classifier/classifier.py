#!/usr/bin/env python
# -*- coding: utf-8 -*-
import threading
from ComssServiceDevelopment.connectors.tcp.msg_stream_connector import InputMessageConnector, OutputMessageConnector #import modułów konektora msg_stream_connector
from ComssServiceDevelopment.connectors.tcp.object_connector import InputObjectConnector, OutputObjectConnector #import modułów konektora object_connector
from ComssServiceDevelopment.service import Service, ServiceController #import modułów klasy bazowej Service oraz kontrolera usługi
import cv2 #import modułu biblioteki OpenCV
import numpy as np #import modułu biblioteki Numpy
import splh
import json

class Filter1Service(Service):
    def __init__(self):
        super(Filter1Service, self).__init__()
        self.filters = []
        self.filters_lock = threading.RLock()

    def declare_outputs(self): #deklaracja wyjść
        self.declare_output("videoOutput", OutputMessageConnector(self))
        self.declare_output("classMatrixOutput", OutputMessageConnector(self))

    def declare_inputs(self):
        self.declare_input("videoInput", InputMessageConnector(self))

    def run(self):

        video_input = self.get_input("videoInput")
        video_output = self.get_output("videoOutput")
        classMatrix_output = self.get_output("classMatrixOutput")

        with open('config.json') as data_file:
            data = json.load(data_file)

        horizontal_density = data["conf"]["horizontal_density"]
        vertical_density = data["conf"]["vertical_density"]
        size = data["conf"]["size"]
        classifier_size = data["conf"]["classifier_size"]

        while self.running(): #pętla główna usługi (wątku głównego obsługującego strumień wideo)
            frame_obj = video_input.read() #odebranie danych z interfejsu wejściowego
            frame = np.loads(frame_obj) #załadowanie ramki do obiektu NumPy

            s = frame.shape

            sx = (s[1]-size)/(horizontal_density-1)
            sy = (s[0]-size)/(vertical_density-1)
                
            #results = []#
            #if 1 in current_filters: #sprawdzenie czy parametr "filtersOn" ma wartość 1, czyli czy ma być stosowany filtr
            #frame = cv2.resize(cv2.getRectSubPix(frame, (256,256), (128*2.5,128*2.5)),(32,32))	#cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY) #zastosowanie filtru COLOR_BGR2GRAY z biblioteki OpenCV na ramce wideo
            
            classMatrix = np.empty([horizontal_density, vertical_density])
 
            for i in range(0,vertical_density):
                for j in range(0,horizontal_density):
                    frame_kiler = cv2.resize(cv2.getRectSubPix(frame, (size,size), (j*sx+size/2,i*sy+size/2)),(classifier_size,classifier_size))#
                    fr = np.hstack(np.hstack(frame_kiler))
                    #if kiler==2:
                    #    cv2.imwrite("image_"+str(i)+"_"+str(j)+".png", frame_kiler)
                    (cls, dist) = splh.classify(fr)
                    classMatrix[j][i] = cls;
                    #results.append((x*j+size/2, y*i+size/2, size, cls))

            
            video_output.send(frame.dumps())
            classMatrix_output.send(classMatrix.dumps())

if __name__=="__main__":
    sc = ServiceController(Filter1Service, "classifier.json") #utworzenie obiektu kontrolera usługi
    sc.start() #uruchomienie usługi
