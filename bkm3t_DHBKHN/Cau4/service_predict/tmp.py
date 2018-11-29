import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import pickle

data_path = 'data/hapt/RawData/'
label = pd.read_csv(data_path+'labels.txt', delimiter=' ', header=None,
                    names=['exp_id', 'user_id', 'act_id', 'start', 'end'])
print(label.head())
windows = []
labels = []
prev_exp = 0
df_acc_exp = None
df_gyro_exp = None
for exp_id, user_id, act_id, start, end in label.values:
    # print(exp_id, user_id)
    if exp_id != prev_exp:
        file_acc = data_path + 'acc_exp{:02}_user{:02}.txt'.format(exp_id, user_id)
        file_gyro = data_path + 'gyro_exp{:02}_user{:02}.txt'.format(exp_id, user_id)

        # print(file_acc)
        # print(file_gyro)

        df_acc_exp = pd.read_csv(file_acc, delimiter=' ', header=None, names=['ax', 'ay', 'az'])
        df_gyro_exp = pd.read_csv(file_gyro, delimiter=' ', header=None, names=['gx', 'gy', 'gz'])
        prev_exp = exp_id
        # print(df_acc_exp.shape, df_gyro_exp.shape)

    acc_win = df_acc_exp.iloc[start:end].values
    gyro_win = df_gyro_exp.iloc[start: end].values
    # print(end - start)

    # print(acc_win)

    win = np.concatenate([acc_win, gyro_win], axis=1)
    windows.append(win)
    labels.append(act_id)
    # return windows, labels

def get_features(list_window):
    features = list()
    for win in list_window:
        max = np.amax(win, axis=0)
        min = np.amin(win, axis=0)
        mean = np.mean(win, axis=0)
        std = np.std(win, axis=0)
        mad = np.median(win, axis=0)
        feature = np.concatenate([max, min, mean, std, mad], axis=0)
        features.append(feature)
    return np.array(features)



# features = get_features(windows)
# print(features.shape)
# print(len(labels))
#
# pickle.dump(features, open('data/features.narray.bin' , 'wb'))
# pickle.dump(np.array(labels), open('data/labels.narray.bin', 'wb'))

features = pickle.load(open('data/features.narray.bin', 'rb'))
print(features.shape)
labels = pickle.load((open('data/labels.narray.bin', 'rb')))
print(labels.shape)

num_train = 1000
x_train = features[:num_train]
y_train = labels[:num_train]
x_test = features[num_train:]
y_test = labels[num_train:]

from sklearn.svm import SVC

model = SVC()
model.fit(x_train, y_train)
print(model.score(x_test, y_test))
predict = model.predict(x_test)
from sklearn.metrics import classification_report, confusion_matrix

print(classification_report(y_test, predict))
lbl = {
    1: 'WALKING',
    2: 'WALKING_UPSTAIRS',
    3: 'WALKING_DOWNSTAIRS',
    4: 'SITTING',
    5: 'STANDING',
    6: 'LAYING',
    7: 'STAND_TO_SIT',
    8: 'SIT_TO_STAND',
    9: 'SIT_TO_LIE',
    10: 'LIE_TO_SIT',
    11: 'STAND_TO_LIE',
    12: 'LIE_TO_STAND'
}

def show_confusion_matrix(true_label, predict):
    conf_mat = confusion_matrix(y_test, predict)

    fig = plt.figure(figsize=(12, 12))
    width = np.shape(conf_mat)[1]
    height = np.shape(conf_mat)[0]

    res = plt.imshow(np.array(conf_mat), cmap=plt.cm.GnBu, interpolation='nearest')
    for i, row in enumerate(conf_mat):
        for j, c in enumerate(row):
            if c > 0:
                plt.text(j - .2, i + .1, c, fontsize=16)

    cb = fig.colorbar(res)
    plt.title('Confusion Matrix')
    _ = plt.xticks(range(12), [l for l in lbl.values()], rotation=90)
    _ = plt.yticks(range(12), [l for l in lbl.values()])
    plt.show()