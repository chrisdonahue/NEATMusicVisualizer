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
import wx

# global state
width = 200
height = 200

def evaluate(genome, audioBuffer, videoBuffer, fitnessType):
    if fitnessType != 'manual':
        net = NEAT.NeuralNetwork()
        genome.BuildPhenotype(net)

        frameCounter = 0
        frameLimit = 100
        while frameCounter < frameLimit:
            audio = audioBuffer.recv()[0]

            if (len(audio) == 0):
                print 'no audio'
                continue

            audioarray = array.array('h')
            audioarray.fromstring(audio)
            
            net.Flush()
            videoData = []
            for x in range(width):
                for y in range(height):
                    net.Input([audioarray[(x + y * width) % len(audioarray)], x, y])
                    net.Activate()
                    net.Activate()
                    net.Activate()
                    o = net.Output()
                    videoData.append((int(o[0] * 255), int(o[1] * 255), int(o[2] * 255)))
            videoBuffer.put(videoData)

            frameCounter = frameCounter + 1
        
        return 10

def evolve(audioBuffer, videoBuffer):       
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
            f = evaluate(g, audioBuffer, videoBuffer, 'automatic')
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

def streamAudio(filePath, audioBuffer):    
    if filePath != "deviceInput":
        f = wave.open(filePath, 'rb')
        sys.stdout.write('%d channels, %d sampling rate\n' % (f.getnchannels(),
                                                              f.getframerate()))

        device = alsaaudio.PCM(card='default')
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

        nframes = 320
        device.setperiodsize(nframes)

        while True:
            audiodata = f.readframes(nframes)
            while audiodata:
                audioBuffer.send([audiodata])
                device.write(audiodata)
                audiodata = f.readframes(nframes)
            f.rewind()
    #else:

class NEATMusicVisualizer(wx.Frame):
    def __init__(self, parent, id, title, size, videoBuffer):
        wx.Frame.__init__(self, parent, id, title, size)
        self.Bind(wx.EVT_IDLE, self.OnIdle)
        self.Centre()
        self.Show(True)
        self.videoBuffer = videoBuffer

    def OnIdle(self, event):
        event.RequestMore(True)
        dc = wx.PaintDC(self)
        videoData = self.videoBuffer.get()
        videoFrame = wx.EmptyImage(width, height, False)
        for i in range(len(videoData)):
            videoFrame.SetRGB(i % width, i / height, videoData[i][0], videoData[i][1], videoData[i][2])
        dc.DrawBitmap(wx.BitmapFromImage(videoFrame), 0, 0, False)

if __name__ == "__main__":
    # start continuous audio stream from either input or sound file
    recvAudio, sendAudio = mpc.Pipe()
    a = mpc.Process(target=streamAudio, args=('tada.wav', sendAudio))
    a.daemon = True
    a.start()

    # initialize the GUI
    app = wx.PySimpleApp()
    videoQueue = mpc.Queue()
    viz = NEATMusicVisualizer(None, -1, 'NEAT Music Visualizer', (width, height), videoQueue)

    # begin NEAT evolution
    n = mpc.Process(target=evolve, args=(recvAudio, videoQueue))
    n.start()
    
    # run GUI loop in main process
    app.MainLoop()
