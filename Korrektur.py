import numpy as np
import matplotlib.pyplot as plt
from sklearn.ensemble import RandomForestRegressor
from sklearn.linear_model import LinearRegression
import statistics
import math

def real(myID):
    x = []
    for i in range(7):
        if(i != myID):
            x.extend([dist(myID,i)] * numberMessurements)
    return x

def dist(one, two):
    return math.sqrt(pow(pos[two][0]-pos[one][0],2)+pow(pos[two][1]-pos[one][1],2))

def getRelevant(i,cutdata):
    output = []
    count = 0
    for j in range(7):
        if(j == i):
            continue
        if(j in anchors):
            output.extend(cutdata[count*100:(count+1)*100])
        count = count +1
    return output

#poistionen der Nodes
pos =[[15,0],[20.39,0],[20.39,7.4],[20.39,21],[15.9,7.25],[0,0],[0,17.9]]

numberMessurements = 100
#welche Nodes sollen als anchor verwendet werden
anchors = [0,1,3,4,5,2]
#welche Node ist die mobile Node
myNumber = 6
#gemessene Entfernungen zu den Anchor Nodes von der Mobile Node
measurements = [[2800,32],[4300,36],[3900,46],[29000,59],[4500,41],[2800,68],[4200,0]]

resultRand = []
resultLin = []

# Die gemessenden Daten der einzelnen Nodes für die Korrektur
data0 = [[]]
data1 = [[]]
data2 = [[]]
data3 = [[]]
data4 = [[]]
data5 = [[]]
data6 = [[]]
data = [data0,data1,data2,data3,data4,data5,data6]

counter = 0
for i in range(7):
    if(i == myNumber):
        continue
    else:
        measurements[i] = [statistics.mean([[j[0] for j in data[myNumber][100*counter : 100 * (counter+1)]]][0]),statistics.mean([[j[1] for j in data[myNumber][100*counter : 100 * (counter+1)]]][0])]
        counter = counter +1

print(measurements)
newdata = [[],[],[],[],[],[],[]]
for i in range(7):
    for j in range(600):
        if(j % 100 < numberMessurements):
            newdata[i].extend([data[i][j]])

for i in range(7):
    if(i in anchors):
        lin = LinearRegression()
        reg = RandomForestRegressor(random_state=0)
        
        datanow = getRelevant(i,newdata[i])
        realnow = getRelevant(i,real(i))
        lin.fit(datanow,realnow)
        reg.fit(datanow,realnow)
        resultLin.extend(lin.predict([measurements[i]]))
        resultRand.extend(reg.predict([measurements[i]]))
    else:
        resultLin.extend([42])
        resultRand.extend([42])

print(resultRand)
print(resultLin)

diffLin = []
diffRand = []

for i in range(len(resultRand)):
    diffLin.extend([resultLin[i] - dist(myNumber,i)])
    diffRand.extend([resultRand[i] - dist(myNumber,i)])

print(diffLin)
print(diffRand)

defaultData = []
diffDefault = []
for i in range(len(measurements)):
    defaultData.extend([measurements[i][0]/100])
    diffDefault.extend([defaultData[i] - dist(myNumber,i)])

print (diffDefault)
sumLin = 0
sumRand = 0

for i in range(len(resultRand)):
    if(i != myNumber):
        sumLin = sumLin + diffLin[i]
        sumRand = sumRand + diffRand[i]

print(sumLin)
print(sumRand)