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
        self.declare_output("objectsOutput", OutputObjectConnector(self))

    def declare_inputs(self):
        self.declare_input("videoInput", InputMessageConnector(self))
        self.declare_input("classMatrixInput", InputMessageConnector(self))

    def run(self):

        video_input = self.get_input("videoInput")
        video_output = self.get_output("videoOutput")
        classMatrix_input = self.get_input("classMatrixInput")
        objects_output = self.get_output("objectsOutput")

        with open('config.json') as data_file:
            data = json.load(data_file)

        horizontal_density = data["conf"]["horizontal_density"]
        vertical_density = data["conf"]["vertical_density"]
        size = data["conf"]["size"]
        classifier_size = data["conf"]["classifier_size"]
        
        prevMat = np.empty([horizontal_density, vertical_density])
        currMat = np.empty([horizontal_density, vertical_density])
        nextMat = np.empty([horizontal_density, vertical_density])
        prevFrame = None
        currFrame = None
        nextFrame = None
        
        for y in range(0,vertical_density):
            for x in range(0,horizontal_density):
                prevMat[x][y] = -1;
                currMat[x][y] = -1;
                nextMat[x][y] = -1;

        while self.running(): #pętla główna usługi (wątku głównego obsługującego strumień wideo)
            
            frame_obj = video_input.read() #odebranie danych z interfejsu wejściowego
            prevFrame = np.loads(frame_obj) #załadowanie ramki do obiektu NumPy

            mat_obj = classMatrix_input.read()
            prevMat = np.loads(mat_obj)
            
            if nextFrame is None:
                nextFrame = currFrame
                currFrame = prevFrame
                nextMat = currMat
                currMat = prevMat
                continue
            
            objects = []
            
            s = currFrame.shape

            sx = (s[1]-size)/(horizontal_density-1)
            sy = (s[0]-size)/(vertical_density-1)
 
            for y in range(0,vertical_density):
                for x in range(0,horizontal_density):
                    prevOK = 0
                    nextOK = 0
                    if currMat[x][y] == -1:
                        continue
                        
                    for dy in range(-1,2):
                        for dx in range(-1,2):
                            if x+dx in range(0,horizontal_density) and y+dy in range(0,vertical_density):
                                prevOK += currMat[x][y] == prevMat[x+dx][y+dy]
                                nextOK += currMat[x][y] == nextMat[x+dx][y+dy]
                    if prevOK and nextOK:
                        objects.append((sx*x, sy*y, size, int(currMat[x][y])))
            
            video_output.send(nextFrame.dumps())
            objects_output.send(objects)
            
            nextFrame = currFrame
            currFrame = prevFrame
            nextMat = currMat
            currMat = prevMat

if __name__=="__main__":
    sc = ServiceController(Filter1Service, "verifier.json") #utworzenie obiektu kontrolera usługi
    sc.start() #uruchomienie usługi
