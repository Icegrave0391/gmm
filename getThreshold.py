import random
import numpy as np

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
    lossValue = 9999999999
    back = []
    fore = []
    for i in background:
        if i[1]+3*i[2] > bMax:
            bMax = i[1]+3*i[2]
    for i in foreground:
        if i < fMin:
            fMin = i
    ratio = backgroundNum/(backgroundNum + len(foreground))
    if foreground:
        fMedium = np.median(foreground)
        fMax = np.max(foreground)
        fAverage = np.average(foreground)
    else:
        fMedium = 100
        fMax = 100
        fAverage = 100
    weightMax = -1
    weightMaxIndex = 0
    for i in range(len(background)):
        if background[i][0] > weightMax:
            weightMaxIndex = i
            weightMax = background[i][0]
    thresholdIter = background[weightMaxIndex][1]
    threshold = background[weightMaxIndex][1] + 3*background[weightMaxIndex][2]
    end = fMax

    while thresholdIter < end:
        back = []
        fore = []
        for i in background:
            if i[1] + 3 * i[2] > thresholdIter:
                back.append(i)
        for i in foreground:
            if i < thresholdIter:
                fore.append(i)
        lossValueNew = loss(thresholdIter, back, fore, bMax, fMin, ratio, lambda1)
        if lossValueNew < lossValue:
            lossValue = lossValueNew
            threshold = thresholdIter
            thresholdIter = thresholdIter + step
        else:
            thresholdIter = thresholdIter + step
    return threshold


def loss(threshold, back, fore, backMax, foreMin, ratio, lambda1):
    lossValue = 0
    for i in back:
        lossValue = lossValue + lambda1 * ratio * i[0] * ((threshold - (i[1] + 3 * i[2])) ** 2)
    for i in fore:
        lossValue = lossValue + (1-lambda1) * (1-ratio) * (1/len(fore)) * (threshold - i)**2
    #if backMax < foreMin:
    lossValue = lossValue + (threshold-backMax)**2 + (threshold - foreMin)**2
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
