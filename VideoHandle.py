from sklearn.mixture import GaussianMixture
import matplotlib.image as mpimg
import matplotlib.pyplot as plt
import sklearn.cluster as skc  # 密度聚类
import numpy as np
import getThreshold
import PickForeground as imghandler
import plot as imgplot
import GTExtract
import argparse
import math

parser = argparse.ArgumentParser(description= 'A Tool for motion detector using GMM')
parser.add_argument("-v",'--video_output_path', type=str, default='./', help='video output path')
parser.add_argument("-n",'--video_frame_num', type=int, default=277, help='frame num for the certain video')
parser.add_argument("-f",'--video_frame_size', type=tuple, default=(1920,1080), help='resolution for the video, in tuple (W_pixel, H_pixel)')
parser.add_argument("-c",'--CTU_size', type=tuple, default=(32,32), help='size for a certain CTU, in tuple (W_pixel, H_pixel)')
parser.add_argument("-g",'--groundtruth_path', type=str, default='ground/test_GT/test1.txt', help='the ground truth file path (.txt file)')
parser.add_argument("-d",'--dataextract_path', type=str, default='datalist/test1/', help='data extract prefix path (without data1.txt)')
parser.add_argument("-i",'--imagesequence_path', type=str, default='media/img/test1/', help='image sequence path to generate video after label')
parser.parse_args()
opt = parser.parse_args()


# set global attributes

frame_size = opt.video_frame_size     #(W_pixel, H_pixel)
ctu_size = opt.CTU_size               #(W_pixel, H_pixel)

N = 3
H = math.ceil(frame_size[1] / ctu_size[1])
W = math.ceil(frame_size[0] / ctu_size[0])
CTU_NUMS = H * W
FRAME_NUMS = opt.video_frame_num

####################PARAMS################################
background = []
backgroundNum = np.zeros(shape=(N, H, W))
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

def matrix(l:list, N, H, W):
    for i in range(N):
        l.append([[[] for i in range(W)] for j in range(H)])
    return l
#save as list
extract_path = opt.dataextract_path
with open(extract_path + 'data1.txt', 'r') as f1:
    f_reaH = f1.read()
    f_data1 = f_reaH.replace("\n", "").split()
    f_data1 = list(map(int, f_data1))
    f_temp1 = list_of_groups(f_data1, CTU_NUMS)
with open(extract_path + 'data2.txt', 'r') as f2:
    f_reaW = f2.read()
    f_data2 = f_reaW.replace("\n", "").split()
    f_data2 = list(map(int, f_data2))
    f_temp2 = list_of_groups(f_data2, CTU_NUMS)
with open(extract_path + 'data3.txt', 'r') as f3:
    f_read3 = f3.read()
    f_data3 = f_read3.replace("\n", "").split()
    f_data3 = list(map(int, f_data3))
    f_temp3 = list_of_groups(f_data3, CTU_NUMS)

f_data = [f_data1, f_data2, f_data3]
f_CTU_data = [f_temp1, f_temp2, f_temp3]
# idx （奇数、模2、模4）
index = [[],[],[]]
for i in range(FRAME_NUMS):
    if i % 2 == 0:
        index[0].append(i)
    else:
        if(i + 1) % 4 == 0:
            index[2].append(i)
        else:
            index[1].append(i)

index[0].pop()
# bg
bg_table = []
bg_table_dbscan = []
fg_table = []
#####################################################
#bg_table : shape(3,4,5)
#bg_table[0,1,2]: 分别是data1 data2 data3的bg
#bg_table[i][j][k] => 每一个对应 CTU_NUMS 个blocks的矩阵
#fg_table 同理
#####################################################
matrix(bg_table, N, H, W)
matrix(bg_table_dbscan, N, H, W)
matrix(fg_table, N, H, W)
#ground_truth
# gt = []
# a = [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]]
# gt.append(a)
# for i in range(2, FRAME_NUMS+1):
#     fpath = './groundtruth'+'/gt00'+str(1103+i)+'.png'
#     I = mpimg.imread(fpath)
#     gt.append(I)
gt_path = opt.groundtruth_path
GTExtracter = GTExtract.GTExtracter(num_frame = FRAME_NUMS, fpath = gt_path, W_pixel = frame_size[0], H_pixel = frame_size[1], ctusize = ctu_size)
gt = GTExtracter.get_gt()

#筛前景 & 整合数据
matrix(img_data, FRAME_NUMS, 0, 0)
for i_idx in range(N):
    data = index[i_idx]
    lens = len(data) #分别处理data1 data2 data3
    for i in range(lens):
        img_data[data[i]] = f_CTU_data[i_idx][i]
        for j in range(H):
            for k in range(W):
                frame = data[i]
                temp_jk = gt[frame][j][k]   #对应块的groundtruth
                if temp_jk == 0:            #bg
                    backgroundNum[i_idx][j][k] += 1
                    temp_jkidx = i * CTU_NUMS + j * W + k
                    bg_table_dbscan[i_idx][j][k].append([f_data[i_idx][temp_jkidx], 0])
                    bg_table[i_idx][j][k].append(f_data[i_idx][temp_jkidx])
                else:
                    temp_jkidx = i * CTU_NUMS + j * W + k
                    fg_table[i_idx][j][k].append(f_data[i_idx][temp_jkidx])


    for k in range(N):
        for i in range(H):
            for j in range(W):
                db = skc.DBSCAN(eps=3, min_samples=3).fit(bg_table_dbscan[k][i][j])  #
                labels = db.labels_
                after_dbscan = np.array(bg_table[k][i][j])
                after_dbscan = after_dbscan[labels != -1]
                after_dbscan = list(after_dbscan)
                bg_table[k][i][j] = after_dbscan

#Gau data
bg_tableFit = np.array(bg_table).reshape(N, H, W)
foreground = np.array(fg_table).reshape(N, H, W)
#print(foreground)

###################################################
#3-D mat （shape = (3,4,5)）元素是list(w, means, sds)
means_t = []
sds_t = []
matrix(means_t, N,H,W)
matrix(sds_t, N,H,W)
###################################################
matrix(background, N, H, W)
#Gaus
n_comp = 5
gmm = GaussianMixture(n_components=n_comp, covariance_type='diag', random_state=0)
for i in range(N):
    for j in range(H):
        for k in range(W):
            gmm.fit(np.array(bg_tableFit[i][j][k]).reshape(-1, 1))
            means_t[i][j][k] = gmm.means_
            sds_t[i][j][k] = gmm.covariances_
            for n in range(n_comp):
                background[i][j][k].append((gmm.weights_[n], float(gmm.means_[n]),float(gmm.covariances_[n])))

# plot test
# means = means_t[0][3][1]
# sds = sds_t[0][3][1]
# x = np.arange(0, 250, 0.1)
# y = np.arange(0, 3, 0.1)
# y0 = []
# y1 = []
# y2 = []
# y3 = []
# y4 = []
# k = []
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

#get threshold
print("++++++++++++++Start Training+++++++++++++++")
threshold = None
ThresHandler = getThreshold.ThresHandler(N=N, H=H, W=W, gmm_compnum=5)
threshold = ThresHandler.get_thres(background)

#plot test
imgplot.show_CTU_fr(11, 37, bg_table, fg_table, threshold)
#judge bg or fg
print("==============Start Img Process================")
frame_info = {'num_frame':FRAME_NUMS, 'H': H, 'W': W}
imgpath_prefix = opt.imagesequence_path
ImgProc = imghandler.ImageProc(frame_info=frame_info, odd_index=index[0], thres=threshold, data=img_data,
                               means=3, sds=1, filter_mode='time', frame_size=(frame_size[0], frame_size[1]), ctu_size=(32, 32), fps=24, imgpath_prefix=imgpath_prefix)
ImgProc.process()
print("=============Finish Img Process================")
