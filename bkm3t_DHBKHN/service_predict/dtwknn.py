from fastdtw import fastdtw
from sklearn.metrics import euclidean_distances
import pandas as pd
import numpy as np
import pickle


class DtwKnn(object):
    def __init__(self, n_neighbors=1, dist=euclidean_distances):
        self.n_neighbors = n_neighbors
        self.dist = dist
        self.templates = []
        self.label = []

    def add_example(self, template, label):
        """
        add single example
        :param template:
        :param label:
        :return:
        """
        self.templates.append(template)
        self.label.append(label)

    def predict(self, x):
        distances = []
        for template, label in zip(self.templates, self.label):
            distance, path = fastdtw(template, x)
            distances.append([distance, label])
        # print(distances)
        df_temp = pd.DataFrame(distances, columns=['distance', 'label'])
        df_temp = df_temp.sort_values('distance', axis=0).values
        statistic = dict()
        for l in self.label:
            if l not in statistic.keys():
                statistic[l] = [0, np.inf]

        for i in range(self.n_neighbors):
            statistic[df_temp[i, 1]][0] += 1
            t = statistic[df_temp[i, 1]][1]
            statistic[df_temp[i, 1]][1] = t if t < df_temp[i, 0] else df_temp[i, 0]

        print(statistic)

        label = list()
        prob = list()

        for key, value in statistic.items():
            label.append(key)
            prob.append(value[0]/(self.n_neighbors * value[1]))
        # print(label, prob)
        idx_max = np.argmax(prob)
        return label[idx_max]

# dt = DtwKnn(n_neighbors=3)
# dt = pickle.load(open('model_saved/dtwknn.model', 'rb'))
# for t, l in zip(dt.templates, dt.label):
#     print(t, l)

# df = pd.read_csv('data/templace.csv', header=None, names=['ax', 'ay', 'az'])
# num_sample = len(df)/20
# print(num_sample)
# for i in range(int(num_sample)):
#     a = df[i*20 : (i+1)*20].values
#     print(a.shape)
#     dt.add_example(a, 'left')
#
#
# pickle.dump(dt, open('model_saved/dtwknn.model', 'wb'))
