TEMPLATE = subdirs
SUBDIRS = Server Client
Server.depends = Client   # 这一行也可以删
