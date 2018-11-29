
from flask import request, Flask, jsonify
import numpy as np
import pandas as pd
import pickle
import json
from preprocess import segment_data, get_features, parse_data, segment_data_with_timestamp
# from gesture import get_gestures
from dtwknn import DtwKnn

mapping_activities = {
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

model_activity = pickle.load(open('model_saved/random_forest.model', 'rb'))
dtwknn = DtwKnn(n_neighbors=5)

app = Flask(__name__)


@app.route('/predict_gesture', methods=['GET', 'POST'])
def predict_gesture():
    if request.method == 'POST':
        data = request.data.decode('utf8')
        # print(data)
        data = parse_data(data)
        # a = data[['ax', 'ay', 'az']]
        # a.to_csv('data/templace.csv', index=False, mode='a', header=False)
        # label = 'left'
        # print(data)
        # print("dtw:", model_gesture.predict(data[['ax', 'ay', 'az']].values))
        #
        # result = get_gestures(data)
        # print("threshold:", result)
        # if result is not None:
        #     return result
        # else:
        #     return 'unknown'
        pass


@app.route('add_sample_dtw', methods=['GET', 'POST'])
def add_sample_dtw():
    if request.method == 'POST':
        data = request.data.decode('utf8')
        data = parse_data(data)
        data_add = data[['ax', 'ay', 'az']].values
        label = 'sitting'
        dtwknn.add_example()


@app.route('/predict_activity', methods=['GET', 'POST'])
def predict_activity():
    if request.method == 'POST':
        data = request.data.decode('utf8')
        data = parse_data(data)
        print(data.shape)
        data_segmented = segment_data_with_timestamp(data, 3) # segment data with window size is 3 seconds
        features = get_features(data_segmented)
        print(features.shape)
        preds = model_activity.predict(features)
        labels = []
        for pred in preds:
            labels.append(mapping_activities[pred])
        return str(labels[-1])
    return 'unknown'


app.run('0.0.0.0', 5000, debug=True)
