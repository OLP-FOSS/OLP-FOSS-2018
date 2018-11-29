import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import preprocess
import pickle
from sklearn.svm import SVC
from sklearn.neighbors import KNeighborsClassifier
from sklearn.neural_network import MLPClassifier
from sklearn.ensemble import RandomForestClassifier
from sklearn import naive_bayes
from utils import show_confusion_matrix



def segment_window(data, time_split):
    results = list()
    timestamps = data[['timestamps']].values
    start_time = timestamps[0]
    start_index = 0
    for idx, current_time in enumerate(timestamps[1:]):
        delta = current_time - start_time
        if delta >= time_split:
            results.append(data.loc[start_index: idx, ['yaw', 'pitch', 'roll', 'ax', 'ay', 'az']].values)
            start_index = idx + 1
            start_time = timestamps[start_index]

    return results


def get_features(data, path_save=None):
    features = list()
    for window in data:
        # window = window[:, 1:]
        max_win = np.amax(window, axis=0)
        min_win = np.amin(window, axis=0)
        mean_win = np.mean(window, axis=0)
        std_win = np.std(window, axis=0)
        mad_win = np.median(window, axis=0)
        feature = np.concatenate([max_win, min_win, mean_win, std_win, mad_win], axis=0)
        features.append(feature)
    if path_save is not None:
        df = pd.DataFrame(features)
        df.to_csv(path_save, header=False, index=False)
    return np.array(features)


def train_naive_bayes(x_train, y_train, x_test, y_test):
    model = naive_bayes.GaussianNB()
    model.fit(x_train, y_train)
    acc = model.score(x_test, y_test)
    print("naive bayes accuracy:", acc)
    pickle.dump(model, open('model_saved/naive_bayes.model', 'wb'))
    return model


def train_svm(x_train, y_train, x_test, y_test):
    model = SVC()
    model.fit(x_train, y_train)
    acc = model.score(x_test, y_test)
    print("svm accuracy:", acc)
    pickle.dump(model, open('model_saved/svm.model', 'wb'))
    return model


def train_knn(x_train, y_train, x_test, y_test):
    model = KNeighborsClassifier()
    model.fit(x_train, y_train)
    acc = model.score(x_test, y_test)
    print("knn accuracy:", acc)
    pickle.dump(model, open('model_saved/knn.model', 'wb'))
    return model


def train_random_forest(x_train, y_train, x_test, y_test):
    model = RandomForestClassifier()
    model.fit(x_train, y_train)
    acc = model.score(x_test, y_test)
    print("random forest accuracy:", acc)
    pickle.dump(model, open('model_saved/randomforest.model', 'wb'))
    return model


def train_mlp(x_train, y_train, x_test, y_test):
    model = MLPClassifier(early_stopping=True)
    model.fit(x_train, y_train)
    acc = model.score(x_test, y_test)
    print("mlp accuracy:", acc)
    pickle.dump(model, open('model_saved/mlp.model', 'wb'))
    return model


def train_full(x_train, y_train, x_test, y_test):
    svm_model = train_svm(x_train, y_train, x_test, y_test)
    knn_model = train_knn(x_train, y_train, x_test, y_test)
    mlp_model = train_mlp(x_train, y_train, x_test, y_test)
    random_forest_model = train_random_forest(x_train, y_train, x_test, y_test)
    naive_bayes_model = train_naive_bayes(x_train, y_train, x_test, y_test)

    plot_confuse_matrix(svm_model, x_test, y_test, 'svm')
    plot_confuse_matrix(knn_model, x_test, y_test, 'knn')
    plot_confuse_matrix(mlp_model, x_test, y_test, 'mlp')
    plot_confuse_matrix(random_forest_model, x_test, y_test, 'random_forest')
    plot_confuse_matrix(naive_bayes_model, x_test, y_test, 'naive_bayes')


def plot_confuse_matrix(model, x_test, y_test, model_name):
    preds = model.predict(x_test)
    show_confusion_matrix(y_test, preds, 'results/' + model_name + '.svg')


def read_data():
    data_path = 'data/hapt/RawData/'
    label = pd.read_csv(data_path+'labels.txt', delimiter=' ', header=None,
                        names=['exp_id', 'user_id', 'act_id', 'start', 'end'])
    # print(label.head())
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
    return windows, labels


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


def split_data_train_test(features, labels, train_rate=0.8, random=True):
    if random:
        index = np.arange(0, len(labels), 1)
        np.random.shuffle(index)
        features = features[index]
        labels = labels[index]

    num_train = int(len(labels) * train_rate)
    x_train = features[:num_train]
    y_train = labels[:num_train]
    x_test = features[num_train:]
    y_test = labels[num_train:]
    return x_train, y_train, x_test, y_test



windows, labels = read_data()
features = get_features(windows)
labels = np.array(labels)
x_train, y_train, x_test, y_test = split_data_train_test(features, labels)
print(x_train.shape, y_train.shape, x_test.shape, y_test.shape)

train_full(x_train, y_train, x_test, y_test)
