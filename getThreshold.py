import random
import numpy as np

class ThresHandler(object):
    """
    A threshold Handler
    """
    def __init__(self, N=3, H=4, W=5, gmm_compnum = 5):
        self.params = {}
        self.params['N'] = N
        self.params['H'] = H
        self.params['W'] = W
        self.params['gmm_compnum'] = gmm_compnum

    def get_thres(self, background):
        """
        Inputs:
        - background: Array of input data of shape(N, H, W)
          background[i,j,k] is given CTU info after GMM :
           a list of tuple [(w, means, sds)] (len = gmm_compnum)


        return:
        - thres: Array of shape(N, H, W)
        """
        N, H, W = self.params['N'], self.params['H'], self.params['W']
        gmm_compnum = self.params['gmm_compnum']
        thres = np.zeros(shape=(N, H, W))
        for i in range(N):
            for j in range(H):
                for k in range(W):
                    bg_info = background[i][j][k]
                    thresholdMax = -1
                    for n in bg_info:
                        thresholdTemp = 1.2*n[1] + 3*n[2]
                        if thresholdTemp > thresholdMax:
                            thresholdMax = thresholdTemp
                    thres[i][j][k] = thresholdMax
        return thres

def getThreshold(background, backgroundNum, foreground, lambda1, step):
    # **************************************************
    # background 是一个list，list里面都是元组（w，mean，sds）
    # backgroundNum 是指背景的个数
    # foreground 是一个list，list中存的是前景的bit值
    # lambda 是一个比例，控制是侧重背景误判为前景还是前景误判为背景
    # step 是步进长度，决定了threshold的取值范围和训练的速度
    # return 需要的threshold
    # **************************************************
    bMax = -1
    fMin = 9999999999
    weightedMiu = 0

    for i in background:
        if i[1] + 3 * i[2] > bMax:
            bMax = i[1] + i[1] + 3 * i[2]
    if foreground:
        fMin = np.min(foreground)
        fMeduim = np.median(foreground)
        fMax = np.max(foreground)
    else:
        fMax = fMin = fMeduim = 3 * bMax
    ratio = backgroundNum / (backgroundNum + len(foreground))
    All = backgroundNum + len(foreground)
    MostWeightedIndex = findTheLargestGroup(background)
    thresholdIter = background[MostWeightedIndex][1] + 3*background[MostWeightedIndex][2]
    for i in background:
        weightedMiu  = weightedMiu + i[0]*(i[1] + 3*i[2])

    threshold = stepByStep(background, foreground, weightedMiu, fMax, bMax, fMin, ratio, All, lambda1, step)
    return threshold

def findTheLargestGroup(background):
    bgMax = -1
    bgIndex = 0
    for i in range(len(background)):
        if background[i][0] > bgMax:
            bgIndex = i
            bgMax = background[i][0]
    return bgIndex


def stepByStep(background, foreground, thresholdIter, end, bMax, fMin, ratio, All, lambda1, step):
    threshold = thresholdIter
    lossValue = 99999999
    while thresholdIter < end:
        back = []
        fore = []
        for i in background:
            if i[1] + 3 * i[2] > thresholdIter:
                back.append(i)
        for i in foreground:
            if i < thresholdIter:
                fore.append(i)
        lossValueNew = loss(thresholdIter, back, fore, bMax, fMin, ratio, All, lambda1)
        if lossValueNew < lossValue:
            lossValue = lossValueNew
            threshold = thresholdIter
            thresholdIter = thresholdIter + step
        else:
            thresholdIter = thresholdIter + step
    return threshold


def loss(threshold, back, fore, backMax, foreMin, ratio, All, lambda1):
    lossValue = 0
    for i in back:
        lossValue = lossValue + lambda1 * ratio * i[0] * ((threshold - (i[1] + 3 * i[2])) ** 2)
    for i in fore:
        lossValue = lossValue + (1-lambda1) * (1-ratio) * (1/len(fore)) * (threshold - i)**2
    lossValue = lossValue + 1/All*((threshold-backMax)**2 + (threshold - foreMin)**2)
    return lossValue

if __name__ == "__main__":
    x = 0.0
    background = []
    foreground = []
    for i in range(5):
        mean = random.uniform(0, 5)
        sds = random.uniform(0, 2)
        background.append((x, mean, sds))
        x = x + 0.1
    for i in range(100):
        foreground.append(random.uniform(0, 100))
    threshold = getThreshold(background, 1000, foreground, 0.5, 0.1)
    print(threshold)
