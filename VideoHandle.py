from sklearn.mixture import GaussianMixture
import matplotlib.image as mpimg
import matplotlib.pyplot as plt
import numpy as np

def normfun(x, mu, sigma):
    p = np.exp(-((x - mu) ** 2)) / (2*sigma ** 2) / (sigma * np.sqrt((2 * np.pi)))
    return p

def matrix(l:list, d0, d1, d2):
    for i in range(d0):
        l.append([[[] for i in range(d2)] for j in range(d1)])
    return l
#save as list
with open('./datalist/data1.txt', 'r') as f1:
    f_read1 = f1.read()
    f_data1 = f_read1.replace("\n", "").split()
    f_data1 = list(map(int, f_data1))
with open('./datalist/data2.txt', 'r') as f2:
    f_read2 = f2.read()
    f_data2 = f_read2.replace("\n", "").split()
    f_data2 = list(map(int, f_data2))
with open('./datalist/data3.txt', 'r') as f3:
    f_read3 = f3.read()
    f_data3 = f_read3.replace("\n", "").split()
    f_data3 = list(map(int, f_data3))
f_data = [f_data1, f_data2, f_data3]

#idx （奇数、模2、模4）
index = [[],[],[]]
for i in range(690):
    if i % 2 == 0:
        index[0].append(i)
    else:
        if(i + 1) % 4 == 0:
            index[2].append(i)
        else:
            index[1].append(i)
#print(index)
#bg
bg_table = []
#############
#bg_table : shape(3,4,5)
#bg_table[0,1,2]: 分别是data1 data2 data3的bg
#bg_table[i][j][k] => 每一个对应20个blocks的矩阵
#############
matrix(bg_table,3, 4, 5)
#ground_truth
gt = []
a = [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]]
gt.append(a)
for i in range(2, 691):
    fpath = './groundtruth'+'/gt00'+str(1103+i)+'.png'
    I = mpimg.imread(fpath)
    gt.append(I)
#print(gt[index[0][1]])
#筛前景
for i_idx in range(3):
    data = index[i_idx]
    #print(data)
    lens = len(data) #分别处理data1 data2 data3
    #print("len: %d" % lens)
    for i in range(lens):
        for j in range(4):
            for k in range(5):
                frame = data[i]
                temp_jk = gt[frame][j][k]   #对应块的groundtruth
                if temp_jk == 0:
                    temp_jkidx = i * 20 + j * 5 + k
                    bg_table[i_idx][j][k].append(f_data[i_idx][temp_jkidx])
#Gau data
bg_tableFit = np.array(bg_table).reshape(3,4,5)
# means_table = []
# sds_table = []
# weights_table = []

background = []
############
#means sds均为3-D mat （shape = (3,4,5)）元素是list, 对应data1/2/3中每个20blocks的参数
############
# matrix(means_table, 3, 4, 5)
# matrix(sds_table, 3, 4, 5)
# matrix(weights_table, 3, 4, 5)
matrix(background, 3, 4, 5)
#Gaus
n_comp = 5
gmm = GaussianMixture(n_components=n_comp, covariance_type='diag', random_state=0)
for i in range(3):
    for j in range(4):
        for k in range(5):
            gmm.fit(np.array(bg_tableFit[i][j][k]).reshape(-1, 1))
            # means_table[i][j][k] = gmm.means_
            # sds_table[i][j][k] = gmm.covariances_
            # weights_table[i][j][k] = gmm.weights_
            for n in range(n_comp):
                background[i][j][k].append((gmm.weights_[n], (gmm.means_[n]), gmm.covariances_[n]))
print(background[0][0][0])
# t_table = [[[] for i in range(5)] for j in range(4)]
# means_table = [[[] for i in range(5)] for j in range(4)]
# sds_table = [[[] for i in range(5)] for j in range(4)]
# for j in range(4):
#     for k in range(5):
#         t_table[j][k] = gmm.fit(bg_tableFit[j][k])
#         means_table[j][k] = gmm.means_[:,1]
#         sds_table[j][k] = gmm.covariances_[:,1]
#         print(sds_table[j][k])
#plot test

# means = means_table[1][3]
# sds = sds_table[1][3]
# y0 = []
# y1 = []
# y2 = []
# y3 = []
# y4 = []
# k = []
# x = np.arange(0, 250, 0.1)
# y = np.arange(0, 3, 0.1)
# thre = 0
# for i in x:
#     y0.append(normfun(i, means[0], sds[0]))
# for i in x:
#     y1.append(normfun(i, means[1], sds[1]))
# for i in x:
#     y2.append(normfun(i, means[2], sds[2]))
# for i in x:
#     y3.append(normfun(i, means[3], sds[3]))
# for i in x:
#     y4.append(normfun(i, means[4], sds[4]))
# for i in range(5):
#     thre = thre + means[i] + 3 * sds[i]
# for i in range(len(y)):
#     k.append(thre)
#
# plt.plot(x, y0, c='b')
# plt.plot(x, y1, c='r')
# plt.plot(x, y2, c='g')
# plt.plot(x, y3, c='m')
# plt.plot(x, y4, c='y')
# plt.plot(k, y, c='c')
# plt.show()