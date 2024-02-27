# dependency is to display the dependency graph
dependency:
	cd build && cmake .. --graphviz=graph.dot  && dot -Tpng graph.dot -o graphimg.png
	cd build ?&& cmake --build .
prepare:
	rm -rf build
	mkdir build
