import math

def matrix(H, W):
    # l = []
    # l.append([[[] for i in range(W)] for j in range(H)])
    return [[0 for i in range(W)] for j in range(H)]

class GTExtracter(object):
    def __init__(self, num_frame = 320, fpath = './datasequence/test15.txt', W_pixel = 1920, H_pixel = 1080, ctusize = (32,32)):
        self.params = {}
        self.params['num_frame'] = num_frame
        self.params['path'] = fpath
        self.params['H_pixel'] = H_pixel
        self.params['W_pixel'] = W_pixel
        self.params['ctu_size'] = ctusize
        self.params['W'] = math.ceil(self.params['W_pixel'] / self.params['ctu_size'][0])
        self.params['H'] = math.ceil(self.params['H_pixel'] / self.params['ctu_size'][1])

    def gt_extract(self):
        path = self.params['path']
        num_frame = self.params['num_frame']
        data_dict = {
            'fg_loc': [],
            'CTU_idx': []
        }
        with open(path) as f:
            for i in range(num_frame):
                # frame line
                fr_line = f.readline().replace('\n', '').split(',')
                while 'null' in fr_line:
                    fr_line.remove('null')
                fr_line = list(map(int, fr_line))
                #print(fr_line)
                # fg num
                fg_num = fr_line[1]
                coords = []
                for num in range(fg_num):
                    start_loc = 3 + 5 * num
                    coords.append(fr_line[start_loc:start_loc + 4])
                #print(coords)
                data_dict['fg_loc'].append(coords)
                # CTUs
                ctus = []
                for ctu_i in range(fg_num):
                    ctu_line = f.readline().replace('\n', '').split(',')
                    while 'CTUs' in ctu_line:
                        ctu_line.remove('CTUs')
                    ctu_line = list(map(int, ctu_line))
                    ctu_line.pop(0)
                    ctus.append(ctu_line)
                data_dict['CTU_idx'].append(ctus)
                # print(ctus)
        return data_dict

    def ctu_2D_idx(self, ctu_idx):
        W = self.params['W']
        return ctu_idx % W, ctu_idx // W

    def gt_to_table(self, data_dict):
        W = self.params['W']
        H = self.params['H']
        # print(W,H)
        ctus = data_dict['CTU_idx']
        gt = []
        for i in range(len(ctus)):
            gt.append(matrix(H, W))
            ctu_curframe = ctus[i]
            for j in range(len(ctu_curframe)):
                ctu_curgroup = ctu_curframe[j]
                for k in range(len(ctu_curgroup)):
                    cur_w, cur_h = self.ctu_2D_idx(ctu_curgroup[k])
                    print(i, ctu_curgroup[k], cur_w, cur_h)
                    gt[i][cur_h][cur_w] = 1
        # print(gt)
        return gt

    # get gt
    def get_gt(self):
        data_dict = self.gt_extract()
        return self.gt_to_table(data_dict)


if __name__ == '__main__':
    GTExtracter = GTExtracter()
    data_dict = GTExtracter.gt_extract()
    table = GTExtracter.gt_to_table(data_dict)