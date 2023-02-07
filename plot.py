import matplotlib.pyplot as plt
import matplotlib as mpl
from cycler import cycler

from pathlib import Path
from collections import defaultdict, deque
from functools import cache

from typing import Union, NamedTuple, Generator

import numpy as np
import math

import csv

DATA_FILE_DIRECTORY = Path('data')
NORMAL_DATA_FILE_NAME =   'n-ns-min-med-max-height.csv'
ZIPZIP_NORMAL_FILE_NAME = 'n-ns-min-med-max-height-tc-ft-bt.csv'
AVG_DEPTHS_DATA_FILE_NAME = 'n-ns-depths-avg.csv'
DEPTHS_DATA_FILE_NAME = 'n-ns-depths.csv'

FIGURE_DIRECTORY = Path('figures')

NUM_NODES_INDEX = 0
TIME_INDEX = 1
MIN_VAL_INDEX = 2
MED_VAL_INDEX = 3
MAX_VAL_INDEX = 4
HEIGHT_INDEX = 5
TOTAL_COMPARISONS_INDEX = 6
FIRST_TIE_INDEX = 7
BOTH_TIE_INDEX = 8

DPI = 300

class FitLine(NamedTuple):
	m: float
	b: float
	x: tuple[int, int]
	y: tuple[float, float]
	r2: float

def binary_search(x: list[int], key: int) -> int:
	low = 0
	high = len(x)

	while low < high:
		mid = (high + low) // 2

		if x[mid] < key:
			low = mid + 1
		else:
			high = mid

	return low

def fit_log_line(x: list[int], y: list[float], skip: int = 0, all_points: bool = False) -> FitLine:
	logx, logy = np.log(x[skip:]), np.log(y[skip:])
	m, b = np.polyfit(logx, logy, 1)
	fit = np.poly1d((m, b))

	expected_y = fit(logx)
	average_y = np.sum(logy) / len(logy)
	r2 = np.sum((expected_y - average_y) ** 2) / np.sum((logy - average_y) ** 2)

	return FitLine(m = m, b = b,
		x = x[skip::] if all_points else x[skip::len(x) - skip - 1],
		y = np.exp(expected_y if all_points else expected_y[::len(expected_y) - 1]),
		r2 = r2)

def plot(title: str, figure_name: str, save: bool, ylabel: str, xlabel: str = 'Num nodes (n)'):
	plt.title(title)
	plt.xlabel(xlabel)
	plt.ylabel(ylabel)

	plt.legend()
	if save:
		path = (FIGURE_DIRECTORY / figure_name).with_suffix('.png')
		plt.savefig(path)
	else:
		plt.show()

@cache
def load_data(ziptree_type: str, val_index: int, file_name: str = NORMAL_DATA_FILE_NAME, min_val: int = 0, max_val: int = 2 ** 28) -> dict[int, list[float]]:
	file = DATA_FILE_DIRECTORY / ziptree_type / file_name

	data = defaultdict(list)

	with file.open('r') as data_file:
		reader = csv.reader(data_file)
		for row in reader:
			n = int(row[NUM_NODES_INDEX])
			if n < min_val or n > max_val:
				continue
			data[n].append(int(row[val_index]))

	return data

@cache
def load_avg_data(ziptree_type: str, val_index: int, file_name: str = NORMAL_DATA_FILE_NAME, min_val: int = 0, max_val: int = 2 ** 28) -> (list[int], list[float]):
	data = load_data(ziptree_type, val_index, file_name = file_name, min_val = min_val, max_val = max_val)
	X, Y = list(), list()

	for x, y in sorted(data.items()):
		X.append(x)
		Y.append(np.mean(y))

	return X, Y

@cache
def load_avg_depths(ziptree_type: str, n: int) -> (list[int], list[float]):
	X = list(range(n))
	Y = list()

	file = DATA_FILE_DIRECTORY / ziptree_type / AVG_DEPTHS_DATA_FILE_NAME

	with file.open('r') as data_file:
		reader = csv.reader(data_file)
		row = next(reader)

		for i in range(n):
			Y.append(float(row[i + 2]))

	return X, Y

@cache
def load_depths(ziptree_type: str, n: int, num_snapshots: int) -> tuple[list[int], list[float]]:
	file = DATA_FILE_DIRECTORY / ziptree_type / DEPTHS_DATA_FILE_NAME

	with file.open('r') as data_file:
		reader = csv.reader(data_file)
		for row in reader:
			if int(row[NUM_NODES_INDEX]) != n:
				continue

			num_snapshots -= 1

			if num_snapshots == 0:
				return list(range(n)), list(float(val) for val in row[2:])

def get_moving_average(arr: list[float], resolution: int = 2 ** 10) -> list[float]:
	total_in_deque = 0
	items = deque()
	new_arr = [0 for _ in range(len(arr))]

	for item in arr[:resolution // 2]:
		items.append(item)
		total_in_deque += item

	for i in range(len(arr)):
		if i >= resolution // 2:
			total_in_deque -= items.popleft()

		if i + resolution // 2 < len(arr):
			items.append(arr[i + resolution // 2])
			total_in_deque += arr[i + resolution // 2]

		new_arr[i] = total_in_deque / len(items)

	return new_arr


def plot_val(ziptree_type: str, val_index: int, label: str, file_name: str = NORMAL_DATA_FILE_NAME, min_val: int = 0, max_val: int = 2 ** 28):
	x, y = load_avg_data(ziptree_type, val_index, file_name = file_name, min_val = min_val, max_val = max_val)
	ylog = [y / math.log(x, 2) for x, y in zip(x, y)]
	plt.loglog(x, ylog, base=2, label = f'Average {label} / lg')
	plt.text(x[-1], ylog[-1], f'{ylog[-1]:.3f}')

def plot_frequency_val(ziptree_type: str, val_index: int, label: str):
	x, y = load_avg_data(ziptree_type, val_index, file_name = ZIPZIP_NORMAL_FILE_NAME, max_val=2**24)
	yfreq = [y / x for x, y in zip(x, y)]
	plt.loglog(x, yfreq, base=2, label = f'# {label} / n')
	plt.text(x[-1], yfreq[-1], f'{yfreq[-1]:g}')

def plot_depths(ziptree_type: str, n: int, label: str = 'Average Depth / lg'):
	x, y = load_avg_depths(ziptree_type, n)
	lg2 = math.log(n, 2)
	ylog = [y1 / lg2 for y1 in y]
	plt.scatter(x, ylog, label = label, linewidths=0, marker = ',', s = (72 / DPI) ** 2)


def plot_depths_snapshot(ziptree_type: str, n: int, num_snapshots: int, label: str = 'Depth / lg'):
	x, y = load_depths(ziptree_type, n, num_snapshots)
	lg2 = math.log(n, 2)
	ylog = [y1 / lg2 for y1 in y]
	plt.scatter(x, ylog, label = label, linewidths=0, marker = ',', s = (72 / DPI) ** 2 * 16)

	y_ma = get_moving_average(y, resolution = 2 ** 10)
	ylog_ma = [y1 / lg2 for y1 in y_ma]
	plt.plot(x, ylog_ma, label = 'Moving Average', linewidth = 1, color = 'black')

def compare_min_max(savefig: bool = False):
	def plot_all_vals(ziptree_type: str):
		plot_val(ziptree_type, MIN_VAL_INDEX, 'Minimum Value', file_name=ZIPZIP_NORMAL_FILE_NAME)
		plot_val(ziptree_type, MAX_VAL_INDEX, 'Maximum Value', file_name=ZIPZIP_NORMAL_FILE_NAME)
		plot_val(ziptree_type, MED_VAL_INDEX, 'Median Value', file_name=ZIPZIP_NORMAL_FILE_NAME)
		plot_val(ziptree_type, HEIGHT_INDEX, 'Height', file_name=ZIPZIP_NORMAL_FILE_NAME)


	plt.figure(num = 0, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_all_vals('original')

	plot('Original Zip-Tree Plot (LogLog), 5k+ simulations', 'original-plot', savefig, 'Depth', 'Num nodes (n)')


	plt.figure(num = 1, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_all_vals('zipzip')

	plot('ZipZip-Tree Plot (LogLog), 5k+ simulations', 'zipzip-plot', savefig, 'Depth', 'Num nodes (n)')

	plt.figure(num = 2, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_all_vals('treap')

	plot('Treap Plot (LogLog), 5k+ simulations', 'treap-plot', savefig, 'Depth', 'Num nodes (n)')


	plt.figure(num = 3, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_val('original', MIN_VAL_INDEX, '(Original) Minimum Value', file_name=ZIPZIP_NORMAL_FILE_NAME)
	plot_val('original', MAX_VAL_INDEX, '(Original) Maximum Value', file_name=ZIPZIP_NORMAL_FILE_NAME)
	plot_val('zipzip', MIN_VAL_INDEX, '(ZipZip) Minimum Value', file_name=ZIPZIP_NORMAL_FILE_NAME)
	plot_val('zipzip', MAX_VAL_INDEX, '(ZipZip) Maximum Value', file_name=ZIPZIP_NORMAL_FILE_NAME)
	plot_val('treap', MIN_VAL_INDEX, '(Treap) Minimum Value', file_name=ZIPZIP_NORMAL_FILE_NAME)
	plot_val('treap', MAX_VAL_INDEX, '(Treap) Maximum Value', file_name=ZIPZIP_NORMAL_FILE_NAME)

	plot('Original vs ZipZip-Tree Edge Values (LogLog), 5k+ simulations', 'comparison-edge', savefig, 'Depth', 'Num nodes (n)')


	plt.figure(num = 4, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_val('original', MED_VAL_INDEX, '(Original) Median Value', file_name=ZIPZIP_NORMAL_FILE_NAME)
	plot_val('original', HEIGHT_INDEX, '(Original) Height', file_name=ZIPZIP_NORMAL_FILE_NAME)
	plot_val('zipzip', MED_VAL_INDEX, '(ZipZip) Median Value', file_name=ZIPZIP_NORMAL_FILE_NAME)
	plot_val('zipzip', HEIGHT_INDEX, '(ZipZip) Height', file_name=ZIPZIP_NORMAL_FILE_NAME)
	plot_val('treap', MED_VAL_INDEX, '(Treap) Median Value', file_name=ZIPZIP_NORMAL_FILE_NAME)
	plot_val('treap', HEIGHT_INDEX, '(Treap) Height', file_name=ZIPZIP_NORMAL_FILE_NAME)

	plot('Original vs ZipMedian and Height Values (LogLog), 5k+ simulations', 'comparison-median-height', savefig, 'Depth', 'Num nodes (n)')

def compare_comparisons(savefig: bool = False):
	plt.figure(num = 301, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_frequency_val('original', TOTAL_COMPARISONS_INDEX, 'Comparisons')
	plot_frequency_val('original', FIRST_TIE_INDEX, 'Geometric Tie')

	plot('ZipZip-Tree Comparisons (LogLog), 5k+ simulations', 'original-comparisons', savefig, 'Frequency', 'Num nodes (n)')

	plt.figure(num = 301, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_frequency_val('zipzip', TOTAL_COMPARISONS_INDEX, 'Comparisons')
	plot_frequency_val('zipzip', FIRST_TIE_INDEX, 'Geometric Tie')
	plot_frequency_val('zipzip', BOTH_TIE_INDEX, 'Both Tie')

	plot('ZipZip-Tree Comparisons (LogLog), 5k+ simulations', 'zipzip-comparisons', savefig, 'Frequency', 'Num nodes (n)')


	plt.figure(num = 302, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_frequency_val('treap', TOTAL_COMPARISONS_INDEX, 'Comparisons')
	plot_frequency_val('treap', FIRST_TIE_INDEX, 'Geometric Tie')

	plot('Treap Comparisons (LogLog), 5k+ simulations', 'treap-comparisons', savefig, 'Frequency', 'Num nodes (n)')


	plt.figure(num = 303, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_frequency_val('original', TOTAL_COMPARISONS_INDEX, 'Original Comparisons')
	plot_frequency_val('original', FIRST_TIE_INDEX, 'Original Geometric Tie')
	plot_frequency_val('zipzip', TOTAL_COMPARISONS_INDEX, 'ZipZip Comparisons')
	plot_frequency_val('zipzip', FIRST_TIE_INDEX, 'ZipZip Geometric Tie')

	plot('Original vs. ZipZip Comparisons (LogLog), 5k+ simulations', 'original-vs-zipzip-comparisons', savefig, 'Frequency', 'Num nodes (n)')

def compare_depths(n: int, savefig: bool = False):
	plt.figure(num = 100, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_depths('original', n)

	plot('Original Zip-Tree Depths, 5k simulations', 'original-depths', savefig, 'Depth', 'Node key')

	plt.figure(num = 101, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_depths('zipzip', n)

	plot('ZipZip-Tree Depths, 5k simulations', 'zipzip-depths', savefig, 'Depth', 'Node key')


	plt.figure(num = 102, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_depths('original', n, '(Original) Average Depth')
	plot_depths('zipzip', n, '(ZipZip) Average Depth')

	plot('Original vs ZipZip-Tree Depths, 5k simulations', 'original-vs-zipzip-depths', savefig, 'Depth', 'Node key')

def compare_depth_snapshots(n: int , savefig: bool):
	for i in range(10):
		plt.figure(num = (200 + i), figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

		plot_depths_snapshot('original', n, i + 1)

		plot(f'Original Zip-Tree (n = {n}) Depths, 1k snapshots', f'original-depths-snapshots-{i + 1}', savefig, 'Depth', 'Node key')

	# plt.figure(num = 201, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	# plot_depths_snapshot('zipzip', n, 1)

	# plot('ZipZip-Tree Depths, 1k snapshots', 'zipzip-depths-snapshots', savefig, 'Depth', 'Node key')

if __name__ == '__main__':
	okabeColors = ['#000000', '#E69F00', '#56B4E9', '#009E73', '#F0E442', '#0072B2', '#D55E00', '#CC79A7']
	plt.rcParams['axes.prop_cycle'] = plt.cycler(color=okabeColors)

	savefig = True
	n = 4194304 // 2 ** 12

	# plt.plot(x, ylog, label = 'Average Depth', marker = '.', linewidth=0.02, markersize = 0.02)
	compare_min_max(savefig)
	compare_comparisons(savefig)
	# compare_depths(n, savefig)
	# compare_depth_snapshots(n, savefig)
