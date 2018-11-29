from flask_api import FlaskAPI
import numpy as np
import pandas as pd
from sklearn.ensemble import RandomForestRegressor
import json
import os
from flask import Flask, flash, request, redirect, url_for
from werkzeug.utils import secure_filename

UPLOAD_FOLDER = './file/'
ALLOWED_EXTENSIONS = set(['csv'])
app = FlaskAPI(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER


@app.route('/predict')
def getYPred():
    xx = request.args.get('x', default = 1, type = float)
    yy = request.args.get('y', default = 1, type = float)
    zz = request.args.get('z', default = 1, type = float)
    dataset = pd.read_csv('./file/data.csv')
    X = dataset.iloc[:, :3].values
    y = dataset.iloc[:, 3].values
    regressor = RandomForestRegressor(n_estimators = 1000, random_state = 0)
    regressor.fit(X, y)
    y_pred = [regressor.predict(np.array([[xx, yy, zz]]))]
    array_in_y_pred = y_pred[0]
    return {
        "message": array_in_y_pred[0]
    }
def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

@app.route('/upload', methods=['GET', 'POST'])
def upload_file():
    if request.method == 'POST':
        # check if the post request has the file part
        if 'file' not in request.files:
            # flash('No file part')
            return {
                "message": 'No file part'
            }
            # return redirect(request.url)
        file = request.files['file']
        # if user does not select file, browser also
        # submit an empty part without filename
        if file.filename == '':
            # flash('No selected file')
            return {
                "message": 'No selected file'
            }
            # return redirect(request.url)
        if file and allowed_file(file.filename):
            filename = secure_filename(file.filename)
            file.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
            return {
                "message": 3
            }
            # return redirect(url_for('uploaded_file',
            #                         filename=filename))


if __name__ == '__main__':
    app.run()