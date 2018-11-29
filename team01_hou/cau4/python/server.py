#!/usr/bin/env python3
'''
'''

import sys
import time
import sched
import threading
import math
import signal
import traceback
import os.path
from collections import deque

import urllib.parse
import urllib.request

import argparse
import math

import pythonosc.dispatcher
import pythonosc.osc_server
import pythonosc.osc_message_builder
import pythonosc.osc_bundle_builder
import pythonosc.udp_client

mtime = lambda: int(round(time.time() * 1000))

#from flask import Flask

activity_current = 0

activity_labels = [
'NoLabel',
'Walking',
'Jogging',
'Cycling',
'Stairs',
'Sitting',
'Standing'
]

config = {
'nodered_host': 'localhost',
'nodered_port': '1880',
'url_api_activity_set': '/api/activity/set',
'running': False,
#'datapath': 'weka/',
'samplerate': 10,
'upsample': True,
'collecttime': 5000,
'samplenum': 0,
'samplemin': 0,
'samplemaxpeaks': 0,
# determine later
'featurenum': 0
}

# 50ms
config['samplenum'] = config['collecttime'] / 1000
if config['upsample']:
	config['samplenum'] = int(config['samplenum'] * config['samplerate'] * 2)
else:
	config['samplenum'] = (config['samplenum'] * config['samplerate'])

config['samplemin'] = int(config['samplenum'] / 2)

config['samplemaxpeaks'] = config['samplenum'] + 1

config['featurenum'] = 44


def train ():
	import arff
	import numpy as np
	import sklearn.tree
	import sklearn.linear_model
	import sklearn.neural_network
	import pandas as pd

	import sklearn.tree
	from sklearn.model_selection import train_test_split
	from sklearn.model_selection import cross_val_predict
	from sklearn.metrics import classification_report
	from sklearn.metrics import confusion_matrix
	from sklearn.metrics import accuracy_score

	import pickle

	# 45 features (X) dataset, additional 1 for class (Y)
	_dataset = arff.load(open('WISDM/WISDM_ar_v1.1_transformed.arff', 'r'))
	_dataset_mask = list()
	i = 0
	while i < 46:
		if i == 1 or i == 0:
			_dataset_mask.append(False)
		else:
			_dataset_mask.append(True)
		i += 1
	# remove unique and user feature for accuracy
	_dataset_mask = np.array(_dataset_mask)
	_dataset = np.array(_dataset['data']) [:, _dataset_mask]
	# slice
	data_x = _dataset[:,:43]
	# slice
	data_y = _dataset[:,43]

	_dataset = None

	clf_dectree = sklearn.tree.DecisionTreeClassifier(random_state=0)
	clf_logreg = sklearn.linear_model.LogisticRegression(C=1e5)
	clf_mlp = sklearn.neural_network.MLPClassifier(random_state=1)
	algo = [clf_dectree, clf_logreg, clf_mlp]

	x_train, x_test, y_train, y_test = train_test_split(data_x, data_y, test_size=0.08, random_state=42)

	x_train = pd.DataFrame(x_train).fillna(0.0).values
	x_test = pd.DataFrame(x_test).fillna(0.0).values

	#clf = clf.fit(data_x, data_y)

	# 3 algorithms
	clf = algo[0].fit(x_train, y_train)
	predictor = cross_val_predict(algo[0], x_train, y_train, cv=10)
	print(classification_report(y_train, predictor, target_names=activity_labels[1:]))
	print(confusion_matrix(y_train, predictor))
	#clf = algo[1].fit(x_train, y_train)
	#predictor = cross_val_predict(algo[1], x_train, y_train, cv=10)
	#clf = algo[2].fit(x_train, y_train)
	#predictor = cross_val_predict(algo[2], x_train, y_train, cv=10)


	with open('model.p', 'wb+') as fp:
		print('Saving to model.p:', fp.write(pickle.dumps(clf)))
		fp.close()

	clfx = pickle.load(open('model.p', 'rb'))

	print('Test saved model.p:', clfx.predict(x_test[0:10]), 'vs', y_test[0:10])

	#nbround = 10
	#kf = KFold(n_splits=nbround)

	#sys.exit(0)


if __name__ == '__main__':
#	parser = argparse.ArgumentParser()
#	parser.add_argument('--ip')
	if not os.path.isfile('model.p'):
		print('model.p does not exists, do training')
		train()

def nodered_send_activity(activity_id, activity_label, mpu_rate):
	return
	with urllib.request.urlopen('http://' + config['nodered_host'] + ':' + config['nodered_port'] + config['url_api_activity_set'] + '?' + urllib.parse.urlencode({
			'activity_id': activity_id,
			'activity_label': activity_label,
			'mpu_rate': mpu_rate,
			'device': 'team01'
		})) as req:
		print('http://' + config['nodered_host'] + ':' + config['nodered_port'] + config['url_api_activity_set'] + '?' + urllib.parse.urlencode({
			'activity_id': activity_id,
			'activity_label': activity_label,
			'mpu_rate': mpu_rate,
			'device': 'team01'
		}))
		print(req.read())
		req.close()
	return

def signal_int_handler(sig, frame):
	print('Exiting!')
	#jvm.stop()
	if rct.is_alive():
		rct.cancel()
	sys.exit(0)
	return

def osc_a_handler(unused_addr, args, a):
	print('[{0}] ~ {1}'.format(args[0], a))
	return

def osc_imu_handler(unused_addr, tm, qw, qx, qy, qz, ax, ay, az, yaw, pitch, roll):
	global harc
	if args.report != 0:
		msg = pythonosc.osc_message_builder.OscMessageBuilder(address='/imu')
		bundle = pythonosc.osc_bundle_builder.OscBundleBuilder(pythonosc.osc_bundle_builder.IMMEDIATELY)
		msg.add_arg(activity_current)
		msg.add_arg(qw)
		msg.add_arg(qx)
		msg.add_arg(qy)
		msg.add_arg(qz)
		msg.add_arg(ax)
		msg.add_arg(ay)
		msg.add_arg(az)
		msg.add_arg(yaw)
		msg.add_arg(pitch)
		msg.add_arg(roll)
		bundle.add_content(msg.build())
		bundle = bundle.build()
		client.send(bundle)

	if config['running']:
		harc.add_data((tm, ax, ay, az, yaw, pitch, roll))

	return

def init_mlframework():
#	global jvm, Classifier, FilteredClassifier, Filter, serialization, loader_for_file
	global sklearn, np
	global HARFE, HARC

#	from weka.core import jvm
#	from weka.core import DenseInstance
#	from weka.core import SerializationHelper
#	from weka.classifiers.meta import FilteredClassifier
#	from weka.filters.unsupervised.attribute import Remove
#	import weka.core.converters.ArffSaver
#	import weka.core.converters.ConverterUtils.DataSource
#	from weka.core.dataset import Instance
#	from weka.core.dataset import Instances
#	from weka.classifiers import Classifier
#	from weka.classifiers import FilteredClassifier
#	from weka.filters import Filter
#	from weka.core import serialization
#	from weka.core.converters import loader_for_file

	import sklearn.tree
	import pickle
	import numpy as np

	class HARFE(object):
		'''
		HARFeatureExtraction
		'''
		bins = (-2.5,0,2.5,5,7.5,10,12.5,15,17.5,20,-2.5,0,2.5,5,7.5,10,12.5,15,17.5,20,-2.5,0,2.5,5,7.5,10,12.5,15,17.5,20)
		#uid = 0
		#activity = activity_labels[0]
		#starttime = 0
		#count = 0
		#timestamps = None
		#accelxs = None
		#accleys = None
		#accelzs = None
		#features = None
		#instance = None

		def __init__ (self, starttime, rawdata):
			self.uid = 0
			self.activity = activity_labels[0]
			self.starttime = starttime
			self.count = len(rawdata)
			self.timestamps = list()
			self.accelxs = list()
			self.accelys = list()
			self.accelzs = list()
			i = 0
			while i < len(rawdata):
				self.timestamps.append(rawdata[i][0])
				self.accelxs.append(rawdata[i][1])
				self.accelys.append(rawdata[i][2])
				self.accelzs.append(rawdata[i][3])
				i += 1
			self.features = None
			#self.cur_instance = None
			return

		def clearraw (self):
			self.timestamps = None
			self.accelxs = None
			self.accelys = None
			self.accelzs = None
			return

		def fillbins (self, n, axis):
			binavgs = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
			bincounts = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
			b = 0
			if axis == 'x':
				b = 0
			elif axis == 'y':
				b = 10
			elif axis == 'z':
				b = 20
			for i in n:
				if i < self.bins[0]:
					bincounts[0] += 1
				elif i < self.bins[b + 1]:
					bincounts[1] += 1
				elif i < self.bins[b + 2]:
					bincounts[2] += 1
				elif i < self.bins[b + 3]:
					bincounts[3] += 1
				elif i < self.bins[b + 4]:
					bincounts[4] += 1
				elif i < self.bins[b + 5]:
					bincounts[5] += 1
				elif i < self.bins[b + 6]:
					bincounts[6] += 1
				elif i < self.bins[b + 7]:
					bincounts[7] += 1
				elif i < self.bins[b + 8]:
					bincounts[8] += 1
				else:
					bincounts[9] += 1

			i = 0
			while i < 10:
				binavgs[i] = float(bincounts[i]) / self.count
				i += 1
				pass
			
			return binavgs

		def getavr (self, n):
			avr = 0.0
			i = 0
			for i in n:
				avr += i
				pass
			avr = avr / len(n)
			return avr

		def getpeaktime (self, t, n):
			allpeaks = list()
			peaktimes = list()
			hightimes = list()
			tmp1 = n[0]
			tmp2 = n[1]
			tmp3 = n[2]
			highpeakcount = 0
			avr = 0.0
			highest = 0.0
			threshold = 0.0
			i = 3
			j = 0
			while i < (self.count - 2):
				if tmp2 > tmp1 and tmp2 > tmp3:
					#allpeaks[j] = tmp2
					allpeaks.append(tmp2)
					#peaktimes[j] = t[i]
					peaktimes.append(t[i])
					if tmp2 > highest:
						highest = tmp2
					j += 1

				tmp1 = tmp2
				tmp2 = tmp3
				tmp3 = n[i + 1]

				i += 1
				pass

			i = 0
			while i < len(allpeaks):
				if allpeaks[i] > (threshold * highest):
					#hightimes[highpeakcount] = peaktimes[i]
					hightimes.append(peaktimes[i])
					highpeakcount += 1
				i += 1
				pass

			while highpeakcount < 3 and threshold > 0.0:
				threshold -= .05
				highpeakcount = 0
				i = 0
				while i < len(allpeaks):
					if allpeaks[i] > (threshold * highest):
						hightimes[highpeakcount] = peaktimes[i]
						highpeakcount += 1
					i += 1
					pass
				pass

			if highpeakcount < 3:
				avr = 0.0
			else:
				i = 0
				while i < (highpeakcount - 1):
					avr += (hightimes[i + 1] - hightimes[i])
					avr = avr / (highpeakcount - 1)
					i += 1
					pass

			return avr

		def getabsdev (self, n, navr):
			adev = 0.0
			for i in n:
				adev += abs(i - navr)
				pass
			adev = adev / len(n)
			return adev

		def getstanddiv (self, n, navr):
			nsdiv = 0.0
			for i in n:
				nsdiv += ((i - navr) * (i - navr))
				pass
			nsdiv = (math.sqrt(nsdiv)) / len(n)
			return nsdiv

		def process (self):
			self.features = list()
			xbins = self.fillbins(self.accelxs, 'x')
			ybins = self.fillbins(self.accelys, 'y')
			zbins = self.fillbins(self.accelzs, 'z')

			i = 0
			while i < 10:
				self.features.append(xbins[i])
				i += 1
				pass
			i = 0
			while i < 10:
				self.features.append(ybins[i])
				i += 1
				pass
			i = 0
			while i < 10:
				self.features.append(zbins[i])
				i += 1
				pass

			self.features.extend([0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0])

			self.features[30] = self.getavr(self.accelxs)
			self.features[31] = self.getavr(self.accelys)
			self.features[32] = self.getavr(self.accelzs)
			self.features[33] = self.getpeaktime(self.timestamps, self.accelxs)
			self.features[34] = self.getpeaktime(self.timestamps, self.accelys)
			self.features[35] = self.getpeaktime(self.timestamps, self.accelzs)
			self.features[36] = self.getabsdev(self.accelxs, self.features[30])
			self.features[37] = self.getabsdev(self.accelys, self.features[31])
			self.features[38] = self.getabsdev(self.accelzs, self.features[32])
			self.features[39] = self.getstanddiv(self.accelxs, self.features[30])
			self.features[40] = self.getstanddiv(self.accelys, self.features[31])
			self.features[41] = self.getstanddiv(self.accelzs, self.features[32])

			# get avrage magnitude
			mags = list()
			magssum = 0.0
			i = 0
			while i < self.count:
				magssum = ((math.pow(self.accelxs[i], 2)) + (math.pow(self.accelys[i], 2)) + (math.pow(self.accelzs[i], 2)))
				mags.append(math.sqrt(magssum))
				i += 1
				pass

			self.features[42] = self.getavr(mags)

			return

		#def features (self):
		#	return self.features

		def instance (self):
			#if self.cur_instance is not None:
			#	return self.cur_instance
			#vals = list()
			#vals.append(instances.attribute(0).add_string_value(str(self.uid)))
			#vals.append(float(0))
			#i = 0
			#while i < config['featurenum']:
			#	vals.append(float(self.features[i]))
			#	i += 1
			#	pass
			#i = instances.class_attribute.index_of(self.activity)
			#vals.append(i)
			#self.cur_instance = Instance.create_instance(values=vals, weight=1.0)
			#self.cur_instance.dataset = instances
			#return self.cur_instance
			return self.features

		#def result (self):
		#	return self.features

	class HARC(object):
		'''
		HARClassifier
		'''
		#cls = None
		#instances = None
		#rawdata = None
		#laststarttime = 0
		#classifying = False

		def __init__ (self):
			global config
			#loader = loader_for_file(config['datapath'] + 'head.arff')
			#self.instances = loader.load_file(config['datapath'] + 'head.arff')
			#if self.instances.class_index == -1:
			#	self.instances.class_index = self.instances.num_attributes - 1
			#self.cls = FilteredClassifier()
			#self.cls.classifier = Classifier(jobject=serialization.read(config['datapath'] + 'j48_new.model'))
			#remove = Filter(classname='weka.filters.unsupervised.attribute.Remove', options=['-R', '1'])
			#remove.inputformat(self.instances)
			#self.cls.filter = remove
			#
			#config['featurenum'] = self.instances.num_attributes - 1 - 1
			#
			self.rawdata = list()
			self.laststarttime = 0
			self.classifying = False
			self.__rate_count = 0
			self.cls = pickle.load(open('model.p', 'rb'))
			return

		@property
		def rate_count (self):
			return self.__rate_count

		@rate_count.setter
		def rate_count (self, value):
			self.__rate_count = value
			return

		def preprocess (self, rawdata):
			fe = HARFE(self.laststarttime, rawdata)
			fe.process()
			fe.clearraw()
			return np.array(fe.instance())

		def classify (self, instance):
			res = 0.0
			i = 0
			print(instance)
			print(self.cls.predict(instance.reshape(1, -1))[0])
			try:
				res = str(self.cls.predict(instance.reshape(1, -1))[0])
				#instance.classvalue(res)
				#if i >= len(activity_labels) or i < 0:
				#	i = 0
				i = activity_labels.index(res)
			except Exception:
				#print(traceback.format_exc())
				return 0
			return i

		def add_data (self, txyzypr):
			global activity_current
			self.__rate_count += 1
			if len(self.rawdata) == 0:
				self.laststarttime = txyzypr[0]
			self.rawdata.append(txyzypr)
			if config['upsample']:
				self.rawdata.append(txyzypr)			
			if (txyzypr[0] - self.laststarttime) > config['collecttime']:
				activity = 0
				if not self.classifying:
					if len(self.rawdata) >= config['samplemin']:
						self.classifying = True

						instance = self.preprocess(self.rawdata)
						activity = self.classify(instance)

						activity = int(activity)
					
						#nodered_send_activity(activity, activity_labels[activity])

						self.classifying = False
				
				activity_current = activity
				print('activity: ' + activity_labels[activity])
				self.laststarttime = txyzypr[0]
				del self.rawdata[:]

			return

#	jvm.logger.setLevel(jvm.logging.WARNING)
#	jvm.start(system_cp=True, packages=True)
#	if not jvm.started:
#		sys.exit(1)

	return

if __name__ == '__main__':
	signal.signal(signal.SIGINT, signal_int_handler)
	signal.signal(signal.SIGTERM, signal_int_handler)

	parser = argparse.ArgumentParser()
	parser.add_argument('--ip',
		default='0.0.0.0', help='Listen IP')
	parser.add_argument('--port',
		type=int, default=8080, help='Listen Port')
	parser.add_argument('--reip',
		default='127.0.0.1', help='Redirect IP')
	parser.add_argument('--report',
		type=int, default=8081, help='Redirect Port')
	args = parser.parse_args()

	dispatcher = pythonosc.dispatcher.Dispatcher()
	dispatcher.map('/a', osc_a_handler, 'A')
	dispatcher.map('/imu', osc_imu_handler)

	client = None
	if args.report != 0:
		client = pythonosc.udp_client.SimpleUDPClient(args.reip, args.report)

	#ThreadingOSCUDPServer
	server = pythonosc.osc_server.BlockingOSCUDPServer((args.ip, args.port), dispatcher)

	print('Loading ML Framework')
	init_mlframework()
	harc = HARC()

	#st = sched.scheduler(time.time, time.sleep)
	def nodered_send_a_rate_counter_func ():
		global rct, harc
		rct = threading.Timer(1.0, nodered_send_a_rate_counter_func)
		rct.start()
		print("rate: ", harc.rate_count)

		nodered_send_activity(activity_current, activity_labels[activity_current], harc.rate_count);

		harc.rate_count = 0
		return

	rct = threading.Timer(1.0, nodered_send_a_rate_counter_func)
	rct.start()

#	def flask_server_thread_func ():
#		global activity_current, activity_labels
#		app = Flask(__name__)
#		@app.route('/')
#		def index ():
#			return '<html><head><meta http-equiv="Refresh" content="1"/><title>Activity</title></head><body><center><b>' + activity_labels[activity_current] + '</b></center></body></html>'
#		app.run(debug=False, use_reloader=False)

#	flt = threading.Thread(target=flask_server_thread_func, args=[])
#	flt.start()

#	print('Serving on {}'.format(server.server_address))
	config['running'] = True

	signal.signal(signal.SIGINT, signal_int_handler)
	signal.signal(signal.SIGTERM, signal_int_handler)

	server.serve_forever()

	#jvm.stop()
	sys.exit(0)



# 500 lines

