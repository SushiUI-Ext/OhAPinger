CXX ?= clang++
ARCH := $(shell uname -m)
VERSION := 1.0.0-sushiui

.PHONY: build

build:
	$(CXX) ohapinger.cpp -O2 -std=c++17 -o ohapinger-$(ARCH)-$(VERSION)
