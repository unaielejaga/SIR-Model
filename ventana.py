import tkinter as tk
import tkinter.ttk as ttk
import array as arr
import matplotlib.pyplot as plt
import numpy as np
import serial
from time import sleep
from tkinter import messagebox


class Main:
        
    def __init__(self, master=None):
        
        # Variables array
        n_s = tk.StringVar()
        t_s = tk.StringVar()
        i0_s = tk.StringVar()
        ffp1_d =tk.DoubleVar()
        ffp2_d =tk.DoubleVar()
        fuga1_d =tk.DoubleVar()
        fuga2_d =tk.DoubleVar()
        dist50_d =tk.DoubleVar()
        dist100_d =tk.DoubleVar()
        lavaManos_d =tk.DoubleVar()
        trata_d =tk.DoubleVar()
        
        #Construccion frame
        self.frame1 = ttk.Frame(master)
        self.label1 = ttk.Label(self.frame1)
        self.label1.configure(text='Numero de Habitantes')
        self.label1.place(anchor='nw', relx='0.04', rely='0.15', x='0', y='0')
        self.n = ttk.Entry(self.frame1, textvariable = n_s)
        self.n.place(anchor='nw', relx='0.04', rely='0.2', x='0', y='0')
        self.label2 = ttk.Label(self.frame1)
        self.label2.configure(anchor='center', text='Simulador SIR')
        self.label2.place(anchor='nw', relx='0.45', rely='0.03', x='0', y='0')
        self.label3 = ttk.Label(self.frame1)
        self.label3.configure(text='Tiempo Sim')
        self.label3.place(anchor='nw', relx='0.04', rely='0.30', x='0', y='0')
        self.t = ttk.Entry(self.frame1, textvariable = t_s)
        self.t.place(anchor='nw', relx='0.04', rely='0.35', x='0', y='0')
        self.label4 = ttk.Label(self.frame1)
        self.label4.configure(text='Infectados Iniciales')
        self.label4.place(anchor='nw', relx='0.04', rely='0.45', x='0', y='0')
        self.i0 = ttk.Entry(self.frame1, textvariable = i0_s)
        self.i0.configure(exportselection='true', font='TkDefaultFont')
        self.i0.place(anchor='nw', relx='0.04', rely='0.5', x='0', y='0')
        self.label5 = ttk.Label(self.frame1)
        self.label5.configure(takefocus=True, text='% Uso FFP1')
        self.label5.place(anchor='nw', relx='0.04', rely='0.60', x='0', y='0')
        self.ffp1 = tk.Scale(self.frame1, variable = ffp1_d)
        self.ffp1.configure(orient='horizontal')
        self.ffp1.place(anchor='nw', relx='0.04', rely='0.65', width='165', x='0', y='0')
        self.label6 = ttk.Label(self.frame1)
        self.label6.configure(text='%Uso FFP2')
        self.label6.place(anchor='nw', relx='0.04', rely='0.75', x='0', y='0')
        self.ffp2 = tk.Scale(self.frame1, variable = ffp2_d)
        self.ffp2.configure(orient='horizontal')
        self.ffp2.place(anchor='nw', relx='0.04', rely='0.8', width='165', x='0', y='0')
        self.label7 = ttk.Label(self.frame1)
        self.label7.configure(text='% Fuga del 1%')
        self.label7.place(anchor='nw', relx='0.4', rely='0.15', x='0', y='0')
        self.fuga1 = tk.Scale(self.frame1, variable = fuga1_d)
        self.fuga1.configure(orient='horizontal')
        self.fuga1.place(anchor='nw', relx='0.4', rely='0.20', width='165', x='0', y='0')
        self.label8 = ttk.Label(self.frame1)
        self.label8.configure(text='% Fuga del 2%')
        self.label8.place(anchor='nw', relx='0.4', rely='0.3', x='0', y='0')
        self.fuga2 = tk.Scale(self.frame1, variable = fuga2_d)
        self.fuga2.configure(orient='horizontal')
        self.fuga2.place(anchor='nw', relx='0.4', rely='0.35', width='165', x='0', y='0')
        self.label9 = ttk.Label(self.frame1)
        self.label9.configure(text='% Lavado Correcto de Manos')
        self.label9.place(anchor='nw', relx='0.4', rely='0.45', x='0', y='0')
        self.lavManos = tk.Scale(self.frame1, variable = lavaManos_d)
        self.lavManos.configure(orient='horizontal')
        self.lavManos.place(anchor='nw', relx='0.4', rely='0.5', width='165', x='0', y='0')
        self.label10 = ttk.Label(self.frame1)
        self.label10.configure(text='% Distancia 50cm')
        self.label10.place(anchor='nw', relx='0.4', rely='0.6', x='0', y='0')
        self.dist50 = tk.Scale(self.frame1, variable = dist50_d)
        self.dist50.configure(orient='horizontal')
        self.dist50.place(anchor='nw', relx='0.4', rely='0.65', width='165', x='0', y='0')
        self.label11 = ttk.Label(self.frame1)
        self.label11.configure(text='% Distancia 1m')
        self.label11.place(anchor='nw', relx='0.4', rely='0.75', x='0', y='0')
        self.dist100 = tk.Scale(self.frame1, variable = dist100_d)
        self.dist100.configure(orient='horizontal')
        self.dist100.place(anchor='nw', relx='0.4', rely='0.8', width='165', x='0', y='0')
        self.label12 = ttk.Label(self.frame1)
        self.label12.configure(text='% Avance Tatamientos')
        self.label12.place(anchor='nw', relx='0.76', rely='0.15', x='0', y='0')
        self.trata = tk.Scale(self.frame1, variable = trata_d)
        self.trata.configure(orient='horizontal')
        self.trata.place(anchor='nw', relx='0.76', rely='0.2', width='165', x='0', y='0')
        
        
        def enviar():
            ser = serial.Serial("/dev/ttyS0", 9600)
            
            if len(n_s.get())!= 0 and len(t_s.get())!= 0 and len(i0_s.get())!= 0:
                
                n = int(n_s.get())
                t = int(t_s.get())
                i0 = int(i0_s.get())
                ffp1 = round(ffp1_d.get())
                ffp2 = round(ffp2_d.get())
                lavaManos = round(lavaManos_d.get())
                dist50 = round(dist50_d.get())
                dist100 = round(dist100_d.get())
                fuga1 = round(fuga1_d.get())
                fuga2 = round(fuga2_d.get())
                trata = round(trata_d.get())
                
                if (ffp1 + ffp2) > 100:
                    messagebox.showinfo(message="Es imposible que haya personas que utilicen las dos mascarillas a la vez", title = "Error")
                elif (ffp1 + ffp2) < (fuga1 + fuga2):
                    messagebox.showinfo(message="Es imposible que haya mas personas con fugas que con mascarillas", title = "Error")
                elif (dist50 + dist100) > 100:
                    messagebox.showinfo(message="Es imposible que haya personas que guarden las dos sitancias a la vez", title = "Error")
                else:
                    array = arr.array('l', [lavaManos, ffp1, ffp2, fuga1, fuga2, dist50, dist100, trata, n, t, i0, -1])
                    enviar = list(map(str, array))
                    cont = 0
                    while cont <= 3:
                        for i in enviar:
                            ser.write((i + ",").encode())
                            sleep(0.001)
                        cont = cont + 1
                    resultados = []
                    valor = 0
                    string = ""
                    while valor!="-3":
                        rd = ser.read()
                        sleep(0.03)
                        data_left = ser.inWaiting()
                        rd += ser.read(data_left)
                        string += rd.decode('utf-8')
                        resultados = string.split(',')
                        valor = resultados[len(resultados)-1]
                    S = []
                    I = []
                    R = []
                    tipo = 0
                    for res in resultados:
                        if tipo == 0:
                            if res == "-1":
                                tipo = 1
                                continue
                            else:
                                S.append(float(res))
                        if tipo == 1:
                            if res == "-2":
                                tipo = 2
                                continue
                            else:
                                I.append(float(res))
                        if tipo == 2:
                            if res == "-3":
                                break
                            else:
                                R.append(float(res))
                    
                    dias = []
                    for dia in range(1,t):
                        dias.append(dia)
                    #Plot
                    dias_array = np.array(dias)
                    S_array = np.array(S)
                    I_array = np.array(I)
                    R_array = np.array(R)
                    plt.plot(dias_array,S_array , label = "S")
                    plt.plot(dias_array, I_array, label = "I")
                    plt.plot(dias_array, R_array, label = "R")
                    plt.xlabel('Tiempo')
                    plt.ylabel('Habitantes')
                    plt.title('Resultados')
                    plt.legend()
                    
                    plt.show()
            else:
                messagebox.showinfo(message="Faltan campos por rellenar", title = "Error")
        
        
        self.enviar = ttk.Button(self.frame1, command = enviar)
        self.enviar.configure(text='Enviar')
        self.enviar.place(anchor='nw', height='80', relx='0.76', rely='0.5', width='120', x='0', y='0')
        self.frame1.configure(cursor='arrow', height='500', width='800')
        self.frame1.place(anchor='nw', x='0', y='0')

        # Main widget
        self.mainwindow = self.frame1
        
        

    def run(self):
        self.mainwindow.mainloop()

if __name__ == '__main__':
    import tkinter as tk
    root = tk.Tk()
    root.geometry("800x500")
    app = Main(root)
    app.run()