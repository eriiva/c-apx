#!/usr/bin/env python3
import apx

        
if __name__ == '__main__':

    node1 = apx.Node('TestNode1')
    node1.append(apx.ProvidePort('TestSignal1','S'))
    node1.append(apx.RequirePort('TestSignal2','S'))
    apx.NodeGenerator().generate('.', node1, includes=['Std_Types.h'])
