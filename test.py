from sklearn.mixture import GaussianMixture
import matplotlib.image as mpimg  # mpimg 用于读取图片
import matplotlib.pyplot as plt
import numpy as np


def normfun(x, mu, sigma):
    pdf = np.exp(-((x - mu) ** 2) / (2 * sigma ** 2)) / (sigma * np.sqrt(2 * np.pi))
    return pdf


with open('./datalist/data1.txt', 'r') as f:
    ff = f.read()

ff1 = ff.replace("\n", "")
ff2 = ff1.split()
x = list(map(int, ff2))
data1Index = []
gt = []
bg24 = []
bg24Fit = []
y0 = []
y1 = []
y2 = []
y3 = []
y4 = []
k = []
thre = 0

for i in range(690):
    if i % 2 == 0:
        data1Index.append(i)

### 读取GroundTruth
a = [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]]
gt.append(a)
for i in range(2, 691):
    filePath = './groundtruth' + '/gt00' + str(1103 + i) + '.png'
    lena = mpimg.imread(filePath)
    gt.append(lena)

### 筛掉前景
for i in range(len(data1Index)):
    temp = gt[data1Index[i]][2][4]
    if temp == 0:
        temp24 = i * 20 + 3 * 5 + 1  ###这个对提取的数据的理解不知道对不对
        bg24.append(x[temp24])
for i in range(len(bg24)):
    bg24Fit.append((i, bg24[i]))

### 对背景做高斯
gmmModel = GaussianMixture(n_components=5, covariance_type='diag', random_state=0)
t = gmmModel.fit(bg24Fit)
means = gmmModel.means_
sds = gmmModel.covariances_
for i in range(5):
    print(sds[i][1])

###画图
x = np.arange(0, 250, 0.1)
y = np.arange(0, 3, 0.1)
for i in x:
    y0.append(normfun(i, means[0][1], sds[0][1]))
for i in x:
    y1.append(normfun(i, means[1][1], sds[1][1]))
for i in x:
    y2.append(normfun(i, means[2][1], sds[2][1]))
for i in x:
    y3.append(normfun(i, means[3][1], sds[3][1]))
for i in x:
    y4.append(normfun(i, means[4][1], sds[4][1]))
for i in range(5):
    thre = thre + means[i][1] + 3 * sds[i][1]
for i in range(len(y)):
    k.append(thre)

plt.plot(x, y0, c='b')
plt.plot(x, y1, c='r')
plt.plot(x, y2, c='g')
plt.plot(x, y3, c='m')
plt.plot(x, y4, c='y')
plt.plot(k, y, c='c')
plt.show()
