#!/usr/bin/env python
# -*- coding: utf-8 -*-
from ComssServiceDevelopment.connectors.tcp.msg_stream_connector import OutputMessageConnector #import modułów konektora msg_stream_connector
from ComssServiceDevelopment.connectors.tcp.object_connector import OutputObjectConnector #import modułów konektora object_connector
from ComssServiceDevelopment.development import DevServiceController #import modułu klasy testowego kontrolera usługi

import cv2 #import modułu biblioteki OpenCV
import Tkinter as tk #import modułu biblioteki Tkinter -- okienka

service_controller = DevServiceController("classifier.json") #utworzenie obiektu kontroletra testowego, jako parametr podany jest plik konfiguracji usługi, do której "zaślepka" jest dołączana
service_controller.declare_connection("videoInput", OutputMessageConnector(service_controller)) #deklaracja interfejsu wyjściowego konektora msg_stream_connector, należy zwrócić uwagę, iż identyfikator musi być zgodny z WEJŚCIEM usługi, do której "zaślepka" jest podłączana
service_controller.declare_connection("filtersOnInput", OutputObjectConnector(service_controller)) #analogicznie jak wyżej dla interfesju sterującego
service_controller.declare_connection("resultsOnInput", OutputObjectConnector(service_controller)) #


def update_all(root, cam, filters):
    read_successful, frame = cam.read() #odczyt obrazu z kamery
    new_filters = set()
    if check1.get()==1: #sprawdzenie czy checkbox był zaznaczony
        new_filters.add(1)

    if filters ^ new_filters:
        filters.clear()
        filters.update(new_filters)
        service_controller.get_connection("filtersOnInput").send(list(filters)) #wysłanie wartości parametru streującego w zależności od checkbox'a

    frame_dump = frame.dumps() #zrzut ramki wideo do postaci ciągu bajtów
    service_controller.get_connection("videoInput").send(frame_dump) #wysłanie danych ramki wideo
    root.update()
    root.after(200, func=lambda: update_all(root, cam, filters))

root = tk.Tk()
root.title("Filters")  #utworzenie okienka

cam = cv2.VideoCapture(0) #"podłączenie" do strumienia wideo z kamerki

#obsługa checkbox'a
check1=tk.IntVar()
checkbox1 = tk.Checkbutton(root, text="Filter 1", variable=check1)
checkbox1.pack()

root.after(0, func=lambda: update_all(root, cam, set())) #dołączenie metody update_all do głównej pętli programu, wynika ze specyfiki TKinter
root.mainloop()
