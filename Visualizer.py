#!/usr/bin/python
import os
import sys

# neat inputs
import time
import random as rnd
import commands as comm
import cv2
import numpy as np
import cPickle as pickle
import MultiNEAT as NEAT
import multiprocessing as mpc

# alsa imports
import wave
import getopt
import alsaaudio
import array

# gui imports
import Tkinter

# global state
width = 200
height = 200
def readAndPlayAudio(filename, pipe):    
    f = wave.open(filename, 'rb')
    sys.stdout.write('%d channels, %d sampling rate\n' % (f.getnchannels(),
                                                          f.getframerate()))
    device = alsaaudio.PCM(card='default')
    
    # Set attributes
    device.setchannels(f.getnchannels())
    device.setrate(f.getframerate())

    # 8bit is unsigned in wav files
    if f.getsampwidth() == 1:
        device.setformat(alsaaudio.PCM_FORMAT_U8)
    # Otherwise we assume signed data, little endian
    elif f.getsampwidth() == 2:
        device.setformat(alsaaudio.PCM_FORMAT_S16_LE)
    elif f.getsampwidth() == 3:
        device.setformat(alsaaudio.PCM_FORMAT_S24_LE)
    elif f.getsampwidth() == 4:
        device.setformat(alsaaudio.PCM_FORMAT_S32_LE)
    else:
        raise ValueError('Unsupported format')

    device.setperiodsize(320)

    audiodata = f.readframes(320)
    while audiodata:
       # Read data from stdin
       pipe.send(audiodata)
       device.write(audiodata)
       audiodata = f.readframes(320)

    pipe.send('STOP')
    f.close()
    print 'done'

def evaluate(genome, queue):
    net = NEAT.NeuralNetwork()
    genome.BuildPhenotype(net)
    
    # do stuff and return the fitness
    net.Flush()

    parent_conn, child_conn = mpc.Pipe()
    p = mpc.Process(target=readAndPlayAudio, args=('tada.wav', child_conn))
    p.daemon = True
    p.start()
    
    frequency = 100
    counter = 0
    posfitness = set()
    colfitness = set()
    audiodata = 'GO'
    while audiodata != 'STOP':
        audioarray = array.array('h')
        audiodata = parent_conn.recv()
        audioarray.fromstring(audiodata)
        for frame in audioarray:
            counter += 1
            if counter % frequency == 0:
                net.Flush()
                for x in range(0,width):
                    for y in range(0,height):
                        net.Input([frame/32768, x, y])
                        net.Activate()
                        net.Activate()
                        net.Activate()
                        o = net.Output()
                        arr = [x, y,
                        int(o[0] * 255),
                        int(o[1] * 255),
                        int(o[2] * 255)]
                        queue.put(arr)
                        colfitness.add((arr[0], arr[1], arr[2]))
         
                #net.Input([frame])
                #net.Activate()
                #net.Activate()
                #net.Activate()
                #o = net.Output()
                #arr = [
                #int(o[0] * width),
                #int(o[1] * height),
                ##int(o[2] * 255),
                #int(o[3] * 255),
                #int(o[4] * 255)]
                #queue.put(arr)
                #posfitness.add((arr[0], arr[1]))
                #colfitness.add((arr[2], arr[3], arr[4]))
    queue.put([-1,-1,-1,-1,-1])

    p.join()
    print len(posfitness) + len(colfitness)
    return len(posfitness) + len(colfitness)

def evolve(queue):       
    params = NEAT.Parameters()
    params.PopulationSize = 3
    params.MutateRemLinkProb = 0
    params.RecurrentProb = 0.2
    params.OverallMutationRate = 1.0
    params.MutateAddLinkProb = 0.5
    params.MutateAddNeuronProb = 0.5
    params.MutateWeightsProb = 0.96
    rng = NEAT.RNG()
    rng.TimeSeed()

    g = NEAT.Genome(0, 3, 0, 3, False, NEAT.ActivationFunction.UNSIGNED_SIGMOID, NEAT.ActivationFunction.UNSIGNED_SIGMOID, 0, params)
    pop = NEAT.Population(g, params, True, 1.0)

    for generation in range(1000):
        genome_list = []
        for s in pop.Species:
            for i in s.Individuals:
                genome_list.append(i)

        for g in genome_list:
            f = evaluate(g, queue)
            g.SetFitness(f)

        ## Parallel processing
        #    pool = mpc.Pool(processes = 4)
        #    fits = pool.map(evaluate, genome_list)
        #    for f,g in zip(fits, genome_list):
        #        g.SetFitness(f)

        print 'Best fitness:', max([x.GetLeader().GetFitness() for x in pop.Species])
        
        # test
        net = NEAT.NeuralNetwork()
        pop.Species[0].GetLeader().BuildPhenotype(net)
        img = np.zeros((250, 250, 3), dtype=np.uint8)
        img += 10
        #NEAT.DrawPhenotype(img, (0, 0, 250, 250), net )

        pop.Epoch()
        print "Generation:", generation

def updateCanvas(queue, guicanvas, root):
    if not queue.empty():
        o = q.get()
        if (o[0] == -1):
            print 'gui cleared'
            guicanvas.create_rectangle(0, 0, width, height, fill='black')
            root.after(1, updateCanvas, queue, guicanvas, root)
            return
        x = o[0]
        y = o[1]
        r = o[2]
        g = o[3]
        b = o[4]
        color = str('#%02x%02x%02x' % (r, g, b))
        guicanvas.create_line(x, y, x+1, y+1, fill=color)
        guicanvas.pack()
    root.after(1, updateCanvas, queue, guicanvas, root)
 
if __name__ == "__main__":
    root = Tkinter.Tk()

    guicanvas = Tkinter.Canvas(root, width=width, height=height, bg='black')
    q = mpc.Queue()
    
    n = mpc.Process(target=evolve, args=(q, ))
    n.start()
    
    guicanvas.pack()
    root.after(1, updateCanvas, q, guicanvas, root)
    root.mainloop()
    n.join()
