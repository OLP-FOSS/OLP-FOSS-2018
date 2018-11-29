
from flask import request, Flask, jsonify
import numpy as np
import pandas as pd
import pickle
import json
from preprocess import segment_data, get_features, parse_data
from gesture import get_gestures

model_activity = pickle.load(open('model_saved/svm.model', 'rb'))
mapping_activity = {
    1: "walking",
    2: "upstairs",
    3: "downstairs",
    4: "sitting",
    5: "standing",
    6: "laying"
}

app = Flask(__name__)


@app.route('/predict_gesture', methods=['GET', 'POST'])
def predict_gesture():
    if request.method == 'POST':
        data = request.data.decode('utf8')
        print(data)
        data = parse_data(data)
        a = data[['ax', 'ay', 'az']]
        a.to_csv('data/templace_right.csv', index=False)
        print(data)
        result = get_gestures(data)
        print(result)
        if result is not None:
            return result
        else:
            return 'unknown'


@app.route('/predict_activity', methods=['GET', 'POST'])
def predict_activity():
    if request.method == 'POST':
        data = request.data.decode('utf8')
        data = parse_data(data)
        features = get_features(segment_data(data[['yaw', 'pitch', 'roll', 'ax', 'ay', 'az']], 2, 10, 0.5))
        print(features.shape)
        return mapping_activity[model_activity.predict(features)[0]]
    return 'unknown'


app.run('0.0.0.0', 5000, debug=True)

