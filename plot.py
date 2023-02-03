import matplotlib.pyplot as plt

from pathlib import Path
from collections import defaultdict
from functools import cache


from typing import Union, NamedTuple, Generator

import numpy as np
import math

import csv

DATA_FILE_DIRECTORY = Path('data')
NORMAL_DATA_FILE_NAME = 'n-ns-min-med-max-height.csv'
AVG_DEPTHS_DATA_FILE_NAME = 'n-ns-depths-avg.csv'
DEPTHS_DATA_FILE_NAME = 'n-ns-depths.csv'

FIGURE_DIRECTORY = Path('figures')

NUM_NODES_INDEX = 0
TIME_INDEX = 1
MIN_VAL_INDEX = 2
MED_VAL_INDEX = 3
MAX_VAL_INDEX = 4
HEIGHT_INDEX = 5

DPI = 600

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
def load_data(ziptree_type: str, val_index: int, min_val: int = 0, max_val: int = 2 ** 24) -> dict[int, list[float]]:
	file = DATA_FILE_DIRECTORY / ziptree_type / NORMAL_DATA_FILE_NAME

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
def load_avg_data(ziptree_type: str, val_index: int) -> (list[int], list[float]):
	data = load_data(ziptree_type, val_index)
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
def load_depths(ziptree_type: str, n: int, num_snapshots: int) -> Generator[tuple[list[int], list[float]], None, None]:
	file = DATA_FILE_DIRECTORY / ziptree_type / DEPTHS_DATA_FILE_NAME

	with file.open('r') as data_file:
		reader = csv.reader(data_file)
		for row in reader:
			if int(row[NUM_NODES_INDEX]) != n:
				continue

			yield list(range(n)), list(float(val) for val in row[2:])
			num_snapshots -= 1

			if num_snapshots == 0:
				break

def plot_val(ziptree_type: str, val_index: int, label: str):
	x, y = load_avg_data(ziptree_type, val_index)
	ylog = [y / math.log(x, 2) for x, y in zip(x, y)]
	plt.loglog(x, ylog, base=2, label = f'Average {label} / lg')
	plt.text(x[-1], ylog[-1], f'{ylog[-1]:.3f}')


def plot_depths(ziptree_type: str, n: int, label: str = 'Average Depth / lg'):
	x, y = load_avg_depths(ziptree_type, n)
	lg2 = math.log(n, 2)
	ylog = [y1 / lg2 for y1 in y]
	plt.scatter(x, ylog, label = label, linewidths=0, marker = ',', s = (72 / DPI) ** 2)


def plot_depths_snapshot(ziptree_type: str, n: int, num_snapshots: int, label: str = 'Depth / lg'):
	for x, y in load_depths(ziptree_type, n, num_snapshots):
		lg2 = math.log(n, 2)
		ylog = [y1 / lg2 for y1 in y]
		plt.scatter(x, ylog, label = label, linewidths=0, marker = ',', s = (72 / DPI) ** 2 * 16)

def compare_min_max(savefig: bool = False):
	def plot_all_vals(ziptree_type: str):
		plot_val(ziptree_type, MIN_VAL_INDEX, 'Minimum Value')
		plot_val(ziptree_type, MAX_VAL_INDEX, 'Maximum Value')
		plot_val(ziptree_type, MED_VAL_INDEX, 'Median Value')
		plot_val(ziptree_type, HEIGHT_INDEX, 'Height')


	plt.figure(num = 0, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_all_vals('original')

	plot('Original Zip-Tree Plot (LogLog), 10k+ simulations', 'original-plot', savefig, 'Depth', 'Num nodes (n)')


	plt.figure(num = 1, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_all_vals('zigzag')

	plot('ZigZag Zip-Tree Plot (LogLog), 10k+ simulations', 'zigzag-plot', savefig, 'Depth', 'Num nodes (n)')


	plt.figure(num = 2, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_val('original', MIN_VAL_INDEX, '(Original) Minimum Value')
	plot_val('original', MAX_VAL_INDEX, '(Original) Maximum Value')
	plot_val('zigzag', MIN_VAL_INDEX, '(ZigZag) Minimum Value')
	plot_val('zigzag', MAX_VAL_INDEX, '(ZigZag) Maximum Value')

	plot('Original vs ZigZag Zip-Tree Edge Values (LogLog), 10k+ simulations', 'original-vs-zigzag-edge', savefig, 'Depth', 'Num nodes (n)')


	plt.figure(num = 3, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_val('original', MED_VAL_INDEX, '(Original) Median Value')
	plot_val('original', HEIGHT_INDEX, '(Original) Height')
	plot_val('zigzag', MED_VAL_INDEX, '(ZigZag) Median Value')
	plot_val('zigzag', HEIGHT_INDEX, '(ZigZag) Height')

	plot('Original vs ZigZag Median and Height Values (LogLog), 10k+ simulations', 'original-vs-zigzag-median-height', savefig, 'Depth', 'Num nodes (n)')

def compare_depths(n: int, savefig: bool = False):
	plt.figure(num = 100, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_depths('original', n)

	plot('Original Zip-Tree Depths, 5k simulations', 'original-depths', savefig, 'Depth', 'Node key')

	plt.figure(num = 101, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_depths('zigzag', n)

	plot('ZigZag Zip-Tree Depths, 5k simulations', 'zigzag-depths', savefig, 'Depth', 'Node key')


	plt.figure(num = 102, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_depths('original', n, '(Original) Average Depth')
	plot_depths('zigzag', n, '(ZigZag) Average Depth')

	plot('Original vs ZigZag Zip-Tree Depths, 5k simulations', 'original-vs-zigzag-depths', savefig, 'Depth', 'Node key')

def compare_depth_snapshots(n: int , savefig: bool):
	plt.figure(num = 200, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_depths_snapshot('original', n, 1)

	plot('Original Zip-Tree Depths, 1k snapshots', 'original-depths-snapshots', savefig, 'Depth', 'Node key')

	plt.figure(num = 201, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_depths_snapshot('zigzag', n, 1)

	plot('ZigZag Zip-Tree Depths, 1k snapshots', 'zigzag-depths-snapshots', savefig, 'Depth', 'Node key')

if __name__ == '__main__':
	savefig = False
	n = 4194304 // 2 ** 12

	# plt.plot(x, ylog, label = 'Average Depth', marker = '.', linewidth=0.02, markersize = 0.02)
	# compare_min_max(savefig)
	# compare_depths(n, savefig)
	compare_depth_snapshots(n, savefig)
