from sklearn.mixture import GaussianMixture
import matplotlib.image as mpimg
import matplotlib.pyplot as plt
import numpy as np
import getThreshold
import PickForeground as imghandler
import plot as imgplot
CTU_NUMS = 20
FRAME_NUMS = 690
d0 = 3
d1 = 4
d2 = 5
####################PARAMS################################
background = []
backgroundNum = np.zeros(shape=(d0, d1, d2))
foreground = []
img_data = []
##########################################################
def list_of_groups(init_list, childern_list_len):
    #init_list为初始化的列表，childern_list_len初始化列表中的几个数据组成一个小列表
    #:param init_list:
    #:param childern_list_len:
    #:return:
    list_of_group = zip(*(iter(init_list),) *childern_list_len)
    end_list = [list(i) for i in list_of_group]
    count = len(init_list) % childern_list_len
    end_list.append(init_list[-count:]) if count !=0 else end_list
    return end_list

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
    f_temp1 = list_of_groups(f_data1, CTU_NUMS)
with open('./datalist/data2.txt', 'r') as f2:
    f_read2 = f2.read()
    f_data2 = f_read2.replace("\n", "").split()
    f_data2 = list(map(int, f_data2))
    f_temp2 = list_of_groups(f_data2, CTU_NUMS)
with open('./datalist/data3.txt', 'r') as f3:
    f_read3 = f3.read()
    f_data3 = f_read3.replace("\n", "").split()
    f_data3 = list(map(int, f_data3))
    f_temp3 = list_of_groups(f_data3, CTU_NUMS)

f_data = [f_data1, f_data2, f_data3]
f_CTU_data = [f_temp1, f_temp2, f_temp3]
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
fg_table = []
###############################################
#bg_table : shape(3,4,5)
#bg_table[0,1,2]: 分别是data1 data2 data3的bg
#bg_table[i][j][k] => 每一个对应20个blocks的矩阵
#fg_table 同理
###############################################
matrix(bg_table, d0, d1, d2)
matrix(fg_table, d0, d1, d2)
#ground_truth
gt = []
a = [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]]
gt.append(a)
for i in range(2, 691):
    fpath = './groundtruth'+'/gt00'+str(1103+i)+'.png'
    I = mpimg.imread(fpath)
    gt.append(I)
#筛前景 & 整合数据
matrix(img_data, FRAME_NUMS, 0, 0)
for i_idx in range(d0):
    data = index[i_idx]
    lens = len(data) #分别处理data1 data2 data3
    for i in range(lens):
        img_data[data[i]] = f_CTU_data[i_idx][i]
        for j in range(d1):
            for k in range(d2):
                frame = data[i]
                temp_jk = gt[frame][j][k]   #对应块的groundtruth
                if temp_jk == 0:            #bg
                    backgroundNum[i_idx][j][k] += 1
                    temp_jkidx = i * CTU_NUMS + j * d2 + k
                    bg_table[i_idx][j][k].append(f_data[i_idx][temp_jkidx])
                else:
                    temp_jkidx = i * CTU_NUMS + j * d2 + k
                    fg_table[i_idx][j][k].append(f_data[i_idx][temp_jkidx])

#Gau data
bg_tableFit = np.array(bg_table).reshape(d0,d1,d2)
foreground = np.array(fg_table).reshape(d0,d1,d2)
print(foreground)
#print(foreground)

###################################################
#3-D mat （shape = (3,4,5)）元素是list(w, means, sds)
means_t = []
sds_t = []
matrix(means_t, 3,4,5)
matrix(sds_t, 3,4,5)
###################################################
matrix(background, d0, d1, d2)
#Gaus
n_comp = 5
gmm = GaussianMixture(n_components=n_comp, covariance_type='diag', random_state=0)
for i in range(d0):
    for j in range(d1):
        for k in range(d2):
            gmm.fit(np.array(bg_tableFit[i][j][k]).reshape(-1, 1))
            means_t[i][j][k] = gmm.means_
            sds_t[i][j][k] = gmm.covariances_
            for n in range(n_comp):
                background[i][j][k].append((gmm.weights_[n], float(gmm.means_[n]),float(gmm.covariances_[n])))

means = means_t[0][0][0]
sds = sds_t[0][0][0]
x = np.arange(0, 250, 0.1)
y = np.arange(0, 3, 0.1)
y0 = []
y1 = []
y2 = []
y3 = []
y4 = []
k = []
thre = 0
for i in x:
    y0.append(normfun(i, means[0], sds[0]))
for i in x:
    y1.append(normfun(i, means[1], sds[1]))
for i in x:
    y2.append(normfun(i, means[2], sds[2]))
for i in x:
    y3.append(normfun(i, means[3], sds[3]))
for i in x:
    y4.append(normfun(i, means[4], sds[4]))
for i in range(5):
    thre = thre + means[i] + 3 * sds[i]
for i in range(len(y)):
    k.append(thre)

plt.plot(x, y0, c='b')
plt.plot(x, y1, c='r')
plt.plot(x, y2, c='g')
plt.plot(x, y3, c='m')
plt.plot(x, y4, c='y')
plt.plot(k, y, c='c')
plt.show()

#get threshold
k_step = 10
k_lembda = 0.6
print("++++++++++++++Start Training+++++++++++++++")
threshold = np.zeros(shape=(d0, d1, d2))
for i in range(d0):
    for j in range(d1):
        for k in range(d2):
            print("training %d %d %d" % (i, j, k))
            threshold[i][j][k] = getThreshold.getThreshold(background[i][j][k], backgroundNum[i][j][k], fg_table[i][j][k], k_lembda, k_step)
#plot test
imgplot.show_CTU_fr(2, 2, bg_table, fg_table, threshold)
#judge bg or fg
print("==============Start Img Process================")
dataAfterFilter = imghandler.GaussianfilterByTime(img_data, FRAME_NUMS, 3, 1)
tags = imghandler.judgeBackOrFore(d1, d2, FRAME_NUMS, dataAfterFilter, threshold, index[0], index[1], index[2])
imghandler.showPictures(d1, d2, FRAME_NUMS, tags)
