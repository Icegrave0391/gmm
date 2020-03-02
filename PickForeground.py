from PIL import Image
import numpy as np
import cv2
import math

class ImageProc(object):
    """
    Image Process
    """
    def __init__(self, frame_info = None, odd_index = None, thres = None, data = None,
                 means = 3, sds = 1, filter_mode = 'time',
                 size = (1920, 1080), fps = 24, imgpath_prefix = './datasequence/test15/'):
        if frame_info is None:
            frame_info = {'num_frame':690, 'H':4, 'W':5}
        if odd_index is None:
            odd_index = []
        self.params = {}
        self.params['frame_info'] = frame_info
        self.params['odd_index'] = odd_index
        self.params['thres'] = thres
        self.params['data'] = data
        self.params['timefilter_means'] = means
        self.params['timefilter_sds'] = sds
        self.params['mode'] = filter_mode
        self.params['means'] = means
        self.params['sds'] = sds
        self.params['size'] = size
        self.params['fps'] = fps
        self.params['imgpath_prefix'] = imgpath_prefix

    def GaussianfilterByTime(self, miu, sds):
        frNum = self.params['frame_info']['num_frame']
        data = self.params['data']
        GaussTemp = []
        dataAfterFilter = data
        dataChanged = 0
        for i in range(1, miu * 2):
            GaussTemp.append(math.exp(-((i - miu) ** 2) / (2 * (sds ** 2))) / (sds * math.sqrt(2 * math.pi)))
        for k in range(20):
            for i in range(2, frNum - 2):
                for j in range(-2, 3):
                    dataChanged = data[i + j][k] * GaussTemp[j + 2]
                dataAfterFilter[i][k] = dataChanged
        return dataAfterFilter

    def judgeBackOrFore(self, data):
        frame_info = self.params['frame_info']
        H, W, num_frame = frame_info['H'], frame_info['W'], frame_info['num_frame']
        odd_index = self.params['odd_index']
        thres = self.params['thres']
        tag = np.zeros([num_frame, H, W])
        for frIndex in range(num_frame):
            for i in range(H):
                for j in range(W):
                    temp = i * W + j
                    if frIndex in odd_index:
                        if data[frIndex][temp] > thres[0][i][j]:
                            tag[frIndex][i][j] = 99
        return tag

    def reviseTagAccordingToOddFrame(self, tag):
        for k in range(0, len(tag) - 1, 2):
            for i in range(len(tag[k])):
                for j in range(len(tag[k][i])):
                    if tag[k][i][j] == 99:
                        tag[k + 1][i][j] = 99.
                    else:
                        tag[k + 1][i][j] = 0.
        return tag

    def showPictures(self, tag):
        frame_info = self.params['frame_info']
        fps = self.params['fps']
        size = self.params['size']
        video = cv2.VideoWriter("VideoTest1.avi", cv2.VideoWriter_fourcc('I', '4', '2', '0'), fps, size)
        for frIndex in range(frame_info['num_frame']):
            imgpath = "media/img/test_images/img_" + str(frIndex).zfill(5) + ".jpg"
            img = np.array(Image.open(imgpath))
            for i in range(frame_info['H']):
                for j in range(frame_info['W']):
                    if tag[frIndex][i][j] == 99:
                        for a in range(64):
                            for b in range(64):
                                img[i * 64 + a, j * 64 + b, 0] = img[i * 64 + a, j * 64 + b, 0] * 0.5 + 255 * 0.5
            video.write(img)
        video.release()
        cv2.destroyAllWindows()

    def process(self):
        # use time filter
        if(self.params['mode'] == 'time'):
            self.params['data'] = self.GaussianfilterByTime(self.params['means'], self.params['sds'])
        data = self.params['data']
        tags = self.judgeBackOrFore(data)
        tags = self.reviseTagAccordingToOddFrame(tags)
        self.showPictures(tags)