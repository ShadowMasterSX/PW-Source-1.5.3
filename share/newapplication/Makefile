
all: clean compile1 generate compile2

finance: clean fcompile1 fgenerate fcompile2

clean:
	ant clean

compile1:
	ant

generate:
	java -cp ./cache.jar:./jio.jar -jar wanmeiaudb.jar table.xml

compile2:
	ant


fcompile1:
	ant finance

fgenerate:
	java -cp ./cache.jar:./jio.jar -jar financeaudb.jar finance.xml

fcompile2:
	ant finance

cache: compile1 generate compile2
	java -cp ./cache.jar:./jio.jar:./wanmeiaudb.jar com.wanmei.db.util.Cache

