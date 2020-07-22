---
layout: default
title: Labeller
---

# Time series data annotation tool

## Live web application

* [Try the live web application](wasm-latest/labeller.html)
* Load a test dataset (File/Load demo or ctrl+d)
* Select "Confirm the label channel"
* Drag the edges of the labels with the right mouse button; or add a new label by right-clicking and dragging in an unlabelled area.

## Purpose and usage

Labeller is a minimalistic labelling tool for files containing time series data.

* Data files are text files organised as a space/newline-separated matrix. Lines are samples. Columns are channels.
* Labels are stored per-sample in a user-specified column. 
* Label segments (instances) are identified by the tool as contiguous samples with identical label.
* The tool allows to add, remove, edit labels.


## Screenshot

![Labeller](/docs/img/ex0_crop.png)

## Source code

The source code is on [github](https://github.com/droggen/Labeller).
