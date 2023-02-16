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

COMBINED_DATA_FILE_DIRECTORY = Path('combineddata')
DATA_FILE_DIRECTORY = Path('data')
NORMAL_DATA_FILE_NAME = 'n-cnt-ns-min-med-max-height.csv'
NORMAL_AVG_DATA_FILE_NAME = 'n-cnt-ns-min-med-max-height-avg.csv'
COMPARISON_DATA_FILE_NAME = 'n-cnt-ns-min-med-max-height-tc-ft-bt.csv'
SQRT_DATA_FILE_NAME = 'n-cnt-ns-min-med-max-height-sqrt.csv'
AVG_DEPTHS_DATA_FILE_NAME = 'n-ns-depths-avg.csv'
DEPTHS_DATA_FILE_NAME = 'n-ns-depths.csv'
TREAP_COMPARISON_FILE_NAME = 'n-ns-min-med-max-height-avg-tc-ft-bt.csv'
NEW_COMPARISON_DATA_FILE_NAME = 'n-cnt-ns-min-med-max-height-avg-tc-ft-bt.csv'
NEW_DYNAMIC_DATA_FILE_NAME = 'n-cnt-ns-min-med-max-height-avg-tc-ft-bt-mb-ab.csv'

FIGURE_DIRECTORY = Path('figures')

NUM_NODES_INDEX = 0
NUM_SIMULATIONS_INDEX = 1
TIME_INDEX = 2
MIN_VAL_INDEX = 3
MED_VAL_INDEX = 4
MAX_VAL_INDEX = 5
HEIGHT_INDEX = 6
AVG_INDEX = 7
SQRT_INDEX = 6

TOTAL_COMPARISONS_INDEX = 7
FIRST_TIE_INDEX = 8
BOTH_TIE_INDEX = 9
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

# next_x = x[-1] * 2
# next_logx = np.log2(next_x)
# x.append(next_x)
# logx = np.append(logx, next_logx)

# expected_y = fit(logx)
# average_y = np.sum(logy) / len(logy)
# r2 = np.sum((expected_y[:-1] - average_y) ** 2) / np.sum((logy - average_y) ** 2)

def fit_log_line(x: list[int], y: list[float], skip: int = 0, all_points: bool = False, add_next: list[float] = []) -> FitLine:
	logx, logy = np.log2(x[skip:]), np.log2(y[skip:])
	m, b = np.polyfit(logx, logy, 1)

	# XX = np.vstack((logx, np.ones_like(logx))).T
	# p_no_offset = np.linalg.lstsq(XX[:, :-1], logy)[0]
	# m, b = p_no_offset[0], 0.0

	fit = np.poly1d((m, b))

	for next_x in add_next:
		next_logx = np.log2(next_x)
		x.append(next_x)
		logx = np.append(logx, next_logx)


	expected_y = fit(logx)
	average_y = np.sum(logy) / len(logy)
	r2 = np.sum((expected_y[:len(expected_y) - len(add_next)] - average_y) ** 2) / np.sum((logy - average_y) ** 2)

	return FitLine(m = m, b = b,
		x = x[skip::] if all_points else x[skip::len(x) - skip - 1],
		y = 2 ** (expected_y if all_points else expected_y[::len(expected_y) - 1]),
		r2 = r2)

def plot(title: str, figure_name: str, save: bool, ylabel: str, xlabel: str = 'Num nodes (n)', legend_loc: str = 'best'):
	plt.title(title)
	plt.xlabel(xlabel)
	plt.ylabel(ylabel)

	plt.legend(loc = legend_loc)
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
			data[n].append(float(row[val_index]))

	return data

@cache
def load_old_avg_data(ziptree_type: str, val_index: int, file_name: str = NORMAL_DATA_FILE_NAME, min_val: int = 0, max_val: int = 2 ** 28) -> (list[int], list[float]):
	data = load_data(ziptree_type, val_index, file_name = file_name, min_val = min_val, max_val = max_val)
	X, Y = list(), list()

	for x, y in sorted(data.items()):
		X.append(x)
		Y.append(np.mean(y))

	return X, Y

@cache
def load_avg_data(ziptree_type: str, val_index: int, file_name: str = NORMAL_DATA_FILE_NAME, min_val: int = 0, max_val: int = 2 ** 28) -> (list[int], list[float]):
	file_path = COMBINED_DATA_FILE_DIRECTORY / ziptree_type / file_name
	X, Y = list(), list()

	with file_path.open('r') as data_file:
		reader = csv.reader(data_file)
		for row in reader:
			n = int(row[NUM_NODES_INDEX])
			if n < min_val or n > max_val:
				continue

			X.append(n)
			Y.append(float(row[val_index]))

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


def plot_val(ziptree_type: str, val_index: int, label: str, file_name: str = NORMAL_DATA_FILE_NAME, min_val: int = 0, max_val: int = 2 ** 28, skip_label: bool = False, combine_with: tuple[float, int] = None):
	x, y = load_avg_data(ziptree_type, val_index, file_name = file_name, min_val = min_val, max_val = max_val)
	ylog = [y / math.log(x, 2) for x, y in zip(x, y)]
	plt.loglog(x, ylog, base=2, label = f'{label} / log')
	final_val = (ylog[-1] + combine_with[0]) / combine_with[1] if combine_with is not None else ylog[-1]

	if not skip_label:
		plt.text(x[-1], final_val, f'{final_val:.3f}')
	return ylog[-1]

def plot_frequency_val(ziptree_type: str, val_index: int, label: str, file_name: str = NEW_COMPARISON_DATA_FILE_NAME, scale_by_comparisons: bool = False, skip_label: bool = False, combine_with: tuple[float, int] = None, max_val: int = 2 ** 40):
	x, y = load_avg_data(ziptree_type, val_index + 1, file_name = file_name, max_val=max_val)
	if scale_by_comparisons:
		xc, yc = load_avg_data(ziptree_type, TOTAL_COMPARISONS_INDEX + 1, file_name = file_name, max_val=max_val)
		yfreq = [y / c for c, y in zip(yc, y)]
		p = plt.loglog(x, yfreq, base=2, label = f'# {label} / comparison')
	else:
		yfreq = [y / x for x, y in zip(x, y)]
		p = plt.loglog(x, yfreq, base=2, label = f'# {label} / insertion')

	uniform_ties = ziptree_type == 'uniform' and val_index == FIRST_TIE_INDEX
	zipzip_both = ziptree_type == 'zipzip' and val_index == BOTH_TIE_INDEX

	if uniform_ties:
		fit = fit_log_line(x[:9], yfreq[:9], add_next=[x[9]], skip = 3)

		plt.loglog(fit.x, fit.y, '--', base = 2,
			# label = f'~  {fit.m:.3} log n + {fit.b:.3}, R^2 = {fit.r2:.5}',
			# label = f'~ {2 ** fit.b:.3} n^{fit.m:.3}, R^2 = {fit.r2:.5}',
			label = f'$\\sim {2 ** fit.b:.3} \\hspace{{0.2}} n^{{{fit.m:.3}}}, R^2 = {fit.r2:.5}$',
			markersize = 6, color = p[-1].get_color())

		plt.text(x[8], yfreq[8], f'{yfreq[8]:.3g}')

	if zipzip_both:
		true_fit_x = x
		true_fit_y = yfreq

		log_x = [math.log(x, 2) for x in x]
		fit = fit_log_line(log_x, yfreq, all_points = True, add_next = [log_x[-1] + 1, log_x[-1] + 2], skip = 7)
		true_fit_x = [2 ** x for x in fit.x]

		# two_to_y = [2 ** y for y in yfreq]
		# fit = fit_log_line(x, two_to_y, all_points = True)
		# true_fit_y = [math.log(y, 2) for y in fit.y]

		plt.loglog(true_fit_x, fit.y, '--', base = 2,
			# label = f'~ {fit.m:.3} log log n + {fit.b:.3}, R^2 = {fit.r2:.5}',
			# label = f'~ {2 ** fit.b:.3} log(n)^{fit.m:.3}, R^2 = {fit.r2:.5}',
			label = f'$\\sim {2 ** fit.b:.3} \\hspace{{0.2}} \\log^{{{fit.m:.3}}}{{n}}, R^2 = {fit.r2:.5}$',
			markersize = 6, color = p[-1].get_color())


	if skip_label:
		return yfreq[-1]

	final_val = (yfreq[-1] + combine_with[0]) / combine_with[1] if combine_with is not None else yfreq[-1]

	if ziptree_type == 'original' and val_index == FIRST_TIE_INDEX:
		plt.text(x[-1], yfreq[-1] + 0.015, f'{final_val:.3g}')
	elif ziptree_type == 'zipzip' and val_index == FIRST_TIE_INDEX:
		plt.text(x[-1], yfreq[-1] - 0.04, f'{final_val:.3g}')
	elif ziptree_type == 'zipzip' and val_index == TOTAL_COMPARISONS_INDEX:
		plt.text(x[-1], yfreq[-1] - 6, f'{final_val:.3g}')
	elif not uniform_ties:
		plt.text(x[-1], yfreq[-1], f'{final_val:.3g}')

	return yfreq[-1]

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
	def plot_all_vals(ziptree_type: str, file_name: str = COMPARISON_DATA_FILE_NAME, combine_small_large: bool = False):
		val = plot_val(ziptree_type, MIN_VAL_INDEX, 'Smallest Key', file_name=file_name, skip_label=combine_small_large)
		plot_val(ziptree_type, MAX_VAL_INDEX, 'Maximum Key', file_name=file_name, combine_with=(val, 2) if combine_small_large else None)
		plot_val(ziptree_type, AVG_INDEX, 'Average', file_name=file_name)
		plot_val(ziptree_type, HEIGHT_INDEX, 'Height', file_name=file_name)


	# plt.figure(num = 0, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	# plot_all_vals('original')

	# plot('Original Zip Tree Plot (LogLog), 10k+ simulations', 'original-plot', savefig, 'Average Depth', 'Num nodes (n)')


	# plt.figure(num = 1, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	# plot_all_vals('zipzip', combine_small_large=True)

	# plot('Zip-Zip Tree Plot (LogLog), 10k+ simulations', 'zipzip-plot', savefig, 'Average Depth', 'Num nodes (n)')


	# plt.figure(num = 2, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	# plot_all_vals('uniform', combine_small_large=True)

	# plot('Uniform Zip Tree Plot (LogLog), 10k+ simulations', 'uniform-plot', savefig, 'Average Depth', 'Num nodes (n)')

	# max_val = 2 ** 228

	# plt.figure(num = 3, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	# plot_val('original', MIN_VAL_INDEX, '(Original) Smallest Key', file_name=NORMAL_DATA_FILE_NAME, max_val=max_val)
	# plot_val('original', MAX_VAL_INDEX, '(Original) Maximum Key', file_name=NORMAL_DATA_FILE_NAME, max_val=max_val)
	# val = plot_val('treap', MIN_VAL_INDEX, '(Uniform) Smallest Key', file_name=NORMAL_DATA_FILE_NAME, skip_label=True, max_val=max_val)
	# val += plot_val('treap', MAX_VAL_INDEX, '(Uniform) Maximum Key', file_name=NORMAL_DATA_FILE_NAME, skip_label=True, max_val=max_val)
	# val += plot_val('zipzip', MIN_VAL_INDEX, '(Zip-Zip) Smallest Key', file_name=NORMAL_DATA_FILE_NAME, skip_label=True, max_val=max_val)
	# val += plot_val('zipzip', MAX_VAL_INDEX, '(Zip-Zip) Maximum Key', file_name=NORMAL_DATA_FILE_NAME, combine_with=(val, 4), max_val=max_val)

	# plot('Depth Discrepancy Comparison (LogLog), 10k+ simulations', 'comparison-edge', savefig, 'Average Depth', 'Num nodes (n)')

	# plt.figure(num = 4, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')


	# plot_val('original', AVG_INDEX, '(Original) Average', file_name=NEW_COMPARISON_DATA_FILE_NAME, max_val = max_val)
	# val = plot_val('uniform', AVG_INDEX, '(Uniform) Average', file_name=NEW_COMPARISON_DATA_FILE_NAME, skip_label=True, max_val = max_val)
	# plot_val('zipzip', AVG_INDEX, '(Zip-Zip) Average', file_name=NEW_COMPARISON_DATA_FILE_NAME, combine_with=(val, 2), max_val = max_val)

	# plot('Average Depth Comparison (LogLog), 10k+ simulations', 'comparison-avg', savefig, 'Average Depth', 'Num nodes (n)')

	# plt.figure(num = 5, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	# plot_val('original', HEIGHT_INDEX, '(Original) Height', file_name=NEW_COMPARISON_DATA_FILE_NAME, max_val = max_val)
	# val = plot_val('uniform', HEIGHT_INDEX, '(Uniform) Height', file_name=NEW_COMPARISON_DATA_FILE_NAME, skip_label=True, max_val = max_val)
	# plot_val('zipzip', HEIGHT_INDEX, '(Zip-Zip) Height', file_name=NEW_COMPARISON_DATA_FILE_NAME, combine_with=(val, 2), max_val = max_val)

	# plot('Height Comparison (LogLog), 10k+ simulations', 'comparison-height', savefig, 'Average Depth', 'Num nodes (n)')

	plt.figure(num = 6, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	# plot_val('original', AVG_INDEX, '(Original) Average', file_name=NORMAL_AVG_DATA_FILE_NAME, max_val = max_val)
	# aval = plot_val('treap', AVG_INDEX, '(Uniform) Average', file_name=NORMAL_AVG_DATA_FILE_NAME, skip_label=True, max_val = max_val)
	# plot_val('zipzip', AVG_INDEX, '(Zip-Zip) Average', file_name=NORMAL_AVG_DATA_FILE_NAME, combine_with=(aval, 2), max_val = max_val)
	# plot_val('original', HEIGHT_INDEX, '(Original) Height', file_name=NORMAL_DATA_FILE_NAME, max_val = max_val)
	# hval = plot_val('treap', HEIGHT_INDEX, '(Uniform) Height', file_name=NORMAL_DATA_FILE_NAME, skip_label=True, max_val = max_val)
	# plot_val('zipzip', HEIGHT_INDEX, '(Zip-Zip) Height', file_name=NORMAL_DATA_FILE_NAME, combine_with=(hval, 2), max_val = max_val)


	plot_val('uniform', MIN_VAL_INDEX, '(Uniform) Average', file_name=NEW_COMPARISON_DATA_FILE_NAME, max_val = 2**22)
	plot_val('dynamic', MIN_VAL_INDEX, '(Dynamic) Average', file_name=NEW_DYNAMIC_DATA_FILE_NAME, max_val = 2**22)
	plot_val('uniform', MAX_VAL_INDEX, '(Uniform) Height', file_name=NEW_COMPARISON_DATA_FILE_NAME, max_val = 2**22)
	plot_val('dynamic', MAX_VAL_INDEX, '(Dynamic) Height', file_name=NEW_DYNAMIC_DATA_FILE_NAME, max_val = 2**22)

	# plot_val('uniform', AVG_INDEX, '(Uniform) Average', file_name=NEW_COMPARISON_DATA_FILE_NAME, max_val = 2**22)
	# plot_val('dynamic', AVG_INDEX, '(Dynamic) Average', file_name=NEW_DYNAMIC_DATA_FILE_NAME, max_val = 2**22)
	# # plot_val('dynamic', AVG_INDEX, '(Buggy Dynamic) Average', file_name='buggy-' + NEW_DYNAMIC_DATA_FILE_NAME, max_val = 2**22)
	# plot_val('uniform', HEIGHT_INDEX, '(Uniform) Height', file_name=NEW_COMPARISON_DATA_FILE_NAME, max_val = 2**22)
	# plot_val('dynamic', HEIGHT_INDEX, '(Dynamic) Height', file_name=NEW_DYNAMIC_DATA_FILE_NAME, max_val = 2**22)
	# plot_val('dynamic', HEIGHT_INDEX, '(Buggy Dynamic) Height', file_name='buggy-' + NEW_DYNAMIC_DATA_FILE_NAME, max_val = 2**22)

	plot('Average Depth and Height Comparison (LogLog), 10k+ simulations', 'comparison-avg-height', savefig, 'Average Depth', 'Num nodes (n)')

	# plt.figure(num = 7, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	# plot_val('original', SQRT_INDEX, '(Original) Sqrt', file_name=SQRT_DATA_FILE_NAME)
	# val = plot_val('treap', SQRT_INDEX, '(Uniform) Sqrt', file_name=SQRT_DATA_FILE_NAME, skip_label=True)
	# plot_val('zipzip', SQRT_INDEX, '(Zip-Zip) Sqrt', file_name=SQRT_DATA_FILE_NAME, combine_with=(val, 2))

	# plot('Sqrt Comparison (LogLog), 10k+ simulations', 'comparison-sqrt', savefig, 'Sqrt', 'Num nodes (n)')

	# max_val = 2 ** 16

	# plt.figure(num = 8, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	# plot_val('zipzip', AVG_INDEX, '(Zip-Zip) Average', file_name=NORMAL_AVG_DATA_FILE_NAME, max_val = max_val)
	# plot_val('zipzip', SQRT_INDEX, '(Zip-Zip) Sqrt', file_name=SQRT_DATA_FILE_NAME, max_val = max_val)
	# plot_val('zipzip', HEIGHT_INDEX, '(Zip-Zip) Height', file_name=NORMAL_DATA_FILE_NAME, max_val = max_val)

	# plot('Zip-Zip Tree Comparison (LogLog), 10k+ simulations', 'comparison-zipzip', savefig, 'Average Depth', 'Num nodes (n)')


def compare_comparisons(savefig: bool = False):
	# plt.figure(num = 301, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	# plot_frequency_val('original', TOTAL_COMPARISONS_INDEX, 'Comparisons')
	# plot_frequency_val('original', FIRST_TIE_INDEX, 'Geometric Tie')

	# plot('Zip-Zip Tree Comparisons (LogLog), 5k+ simulations', 'original-comparisons', savefig, 'Frequency', 'Num nodes (n)')

	# plt.figure(num = 301, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	# plot_frequency_val('zipzip', TOTAL_COMPARISONS_INDEX, 'Comparisons')
	# plot_frequency_val('zipzip', FIRST_TIE_INDEX, 'Geometric Tie')
	# plot_frequency_val('zipzip', BOTH_TIE_INDEX, 'Both Tie')

	# plot('Zip-Zip Tree Comparisons (LogLog), 5k+ simulations', 'zipzip-comparisons', savefig, 'Frequency', 'Num nodes (n)')


	# plt.figure(num = 302, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	# plot_frequency_val('uniform', TOTAL_COMPARISONS_INDEX, 'Comparisons')
	# plot_frequency_val('uniform', FIRST_TIE_INDEX, 'Geometric Tie')

	# plot('Treap Comparisons (LogLog), 5k+ simulations', 'uniform-comparisons', savefig, 'Frequency', 'Num nodes (n)')


	max_val = 2 ** 22

	plt.figure(num = 303, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	# plot_frequency_val('original', TOTAL_COMPARISONS_INDEX, 'Original Comparisons', max_val=max_val)
	plot_frequency_val('original', FIRST_TIE_INDEX, 'Original Geometric Tie', max_val=max_val)
	# val = plot_frequency_val('uniform', TOTAL_COMPARISONS_INDEX, 'Uniform Comparisons', skip_label = True, max_val=max_val)
	plot_frequency_val('uniform', FIRST_TIE_INDEX, 'Uniform Tie', scale_by_comparisons = True, max_val=max_val)
	# plot_frequency_val('zipzip', TOTAL_COMPARISONS_INDEX, 'Zip-Zip Comparisons', combine_with=(val, 2), max_val=max_val)
	plot_frequency_val('zipzip', FIRST_TIE_INDEX, 'Zip-Zip Geometric Tie', max_val=max_val)
	plot_frequency_val('zipzip', BOTH_TIE_INDEX, 'Zip-Zip Both Ties', max_val=max_val)

	plot('# of Rank Comparisons Comparison (LogLog), 10k+ simulations', 'sequential-comparison-comparisons', savefig, 'Frequency', 'Num nodes (n)', legend_loc = 'center left')

def compare_depths(n: int, savefig: bool = False):
	plt.figure(num = 100, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_depths('original', n)

	plot('Original Zip Tree Depths, 5k simulations', 'original-depths', savefig, 'Depth', 'Node key')

	plt.figure(num = 101, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_depths('zipzip', n)

	plot('Zip-Zip Tree Depths, 5k simulations', 'zipzip-depths', savefig, 'Depth', 'Node key')


	plt.figure(num = 102, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	plot_depths('original', n, '(Original) Average Depth')
	plot_depths('zipzip', n, '(Zip-Zip) Average Depth')

	plot('Original vs Zip-Zip Tree Depths, 5k simulations', 'original-vs-zipzip-depths', savefig, 'Depth', 'Node key')

def compare_depth_snapshots(n: int , savefig: bool):
	for i in range(10):
		plt.figure(num = (200 + i), figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

		plot_depths_snapshot('original', n, i + 1)

		plot(f'Original Zip Tree (n = {n}) Depths, 1k snapshots', f'original-depths-snapshots-{i + 1}', savefig, 'Depth', 'Node key')

	# plt.figure(num = 201, figsize = (8, 5), dpi = DPI, facecolor = 'w', edgecolor = 'k')

	# plot_depths_snapshot('zipzip', n, 1)

	# plot('Zip-Zip Tree Depths, 1k snapshots', 'zipzip-depths-snapshots', savefig, 'Depth', 'Node key')

if __name__ == '__main__':
	okabeColors = ['#000000', '#E69F00', '#56B4E9', '#009E73', '#F0E442', '#0072B2', '#D55E00', '#CC79A7']
	plt.rcParams['axes.prop_cycle'] = plt.cycler(color=okabeColors)

	savefig = False
	# n = 4194304 // 2 ** 12

	# plt.plot(x, ylog, label = 'Average Depth', marker = '.', linewidth=0.02, markersize = 0.02)
	compare_min_max(savefig)
	# compare_comparisons(savefig)
	# compare_depths(n, savefig)
	# compare_depth_snapshots(n, savefig)
