#include "reactive2.h"
#include <QJsonArray>
#include <QDateTime>
#include <QtSql/QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <iostream>

const auto db_fields = rea::Json(
    "id", "int",
    "timestamp", "timestamp",
    "qrcode", "varchar(255)",
    "carryid", "bigint",
    "inspectinfo", "varchar(4095)");

const auto db_config = rea::Json(
    "host", "0.0.0.0",
    "port", 3306,
    "user", "root",
    "pswd", "1234",
    "db", "operatesql",
    "table", "deepsight",
    "table2", "rrd",
    "limit", "100000"
);

bool insertRecord(const QSqlDatabase& aDB, const QString& aTable, const QJsonObject& aRecord){
    QSqlQuery query(aDB);
    QString keys = "", vals = "";
    for (auto i : aRecord.keys()){
        if (keys != ""){
            keys += ", ";
            vals += ", ";
        }
        keys += i;
        vals += ":" + i;
    }

    query.prepare("replace into " + aTable + "\
    (" + keys + ")\
    values(" + vals + ")");
    for (auto i : aRecord.keys())
        if (aRecord.value(i).isString())
            query.bindValue(":" + i, aRecord.value(i).toString());
        else if (aRecord.value(i).isDouble())
            query.bindValue(":" + i, aRecord.value(i).toInt());
    return query.exec();
}


QJsonArray accessRecord(const QSqlDatabase& aDB, const QString& aTable, const QString& aField, int aValue){
    QJsonArray ret;

    QSqlQuery query(aDB);
    query.prepare("select * from " + aTable + " where " + aField + " = :value");
    query.bindValue(":value", aValue);
    query.exec();
    QSqlRecord rec = query.record();
    while (query.next()) {
        rec = query.record();
        QJsonObject item;
        for (auto i : db_fields.keys()){
            item.insert(i, query.value(rec.indexOf(i)).toString());
        }
        ret.push_back(item);
    }

    return ret;
}

bool tryPrepareTable(QSqlDatabase& sql){

    //需要进行判断默认的连接名是否存在，
    //如果不存在才使用addDatabase()方法，如果存在则使用database()方法
    if(QSqlDatabase::contains("qt_sql_default_connection"))
        sql = QSqlDatabase::database("qt_sql_default_connection");
    else
        sql = QSqlDatabase::addDatabase("QMYSQL", "test");
    //连接数据库
    sql.setHostName(db_config.value("host").toString()); //数据库服务器IP
    sql.setPort(db_config.value("port").toInt());
    sql.setUserName(db_config.value("user").toString());//用户名
    sql.setPassword(db_config.value("pswd").toString());//密码
                            // db.setDatabaseName("test");//使用的数据库
    //打开数据库
    if(!sql.open())//数据库打开失败
    {
        auto test = sql.lastError().text();

        return false;
    }
    QString querystring = "CREATE DATABASE IF NOT EXISTS " + db_config.value("db").toString();
    sql.exec(querystring);
    if (sql.lastError().isValid())
    {
        auto test = sql.lastError().text();
        return false;
    }

    sql.setDatabaseName(db_config.value("db").toString());
    if(!sql.open())
    {
        auto test = sql.lastError().text();
        return false;
    }

    QString fields = "";
    for (auto i : db_fields.keys()){
        if (fields != "")
            fields += ",";
        fields += i + " " + db_fields.value(i).toString();
        if (i == "id")
            fields += " AUTO_INCREMENT NOT NULL UNIQUE";
    }

    querystring = "CREATE TABLE IF NOT EXISTS " + db_config.value("table").toString() + " (" + fields + ",key(id))";
    sql.exec(querystring);
    if (sql.lastError().isValid())
    {
        auto test = sql.lastError().text();
        return false;
    }

    querystring = "CREATE TABLE IF NOT EXISTS " + db_config.value("table2").toString() + " (counter int, id int PRIMARY KEY)";
    sql.exec(querystring);
    if (sql.lastError().isValid())
    {
        auto test = sql.lastError().text();
        return false;
    }

    auto ret = accessRecord(sql, db_config.value("table2").toString(), "id", 0);
    if (ret.size() == 0){
        auto test = insertRecord(sql, db_config.value("table2").toString(), rea::Json("id", 0, "counter", 0));

        querystring = "DROP TRIGGER IF EXISTS " + db_config.value("table").toString() +".trigger_rrd";
        sql.exec(querystring);
        if (sql.lastError().isValid())
        {
            auto test = sql.lastError().text();
            return false;
        }

        querystring = "create trigger trigger_rrd \n\
        BEFORE INSERT ON " + db_config.value("table").toString() + " FOR EACH ROW \n\
        BEGIN \n\
        DECLARE var int; \n\
        DECLARE mesg varchar(10); \n\
        SELECT counter INTO var FROM " + db_config.value("table2").toString() + " where id = 0;\n\
        SET var = var + 1; \n\
        IF var > " + db_config.value("limit").toString() + " THEN\n\
        SET NEW.id = var % " + db_config.value("limit").toString() + ";\n\
        IF NEW.id = 0 THEN \n\
        SET NEW.id = " + db_config.value("limit").toString() + ";\n\
        END IF;\n\
        ELSE \n\
        SET mesg='success'; \n\
        END IF;\n\
        UPDATE " + db_config.value("table2").toString() + " SET counter=counter + 1 where id = 0;\n\
        END";
        std::cout << querystring.toStdString() << std::endl;
        sql.exec(querystring);
        if (sql.lastError().isValid())
        {
            auto test = sql.lastError().text();
            return false;
        }
    }
    return true;
}

bool dropTable(const QSqlDatabase& aDB){
    QString querystring = "DROP TABLE " + db_config.value("table").toString();
    aDB.exec(querystring);
    if (aDB.lastError().isValid())
    {
        auto test = aDB.lastError().text();
        return false;
    }

    querystring = "DROP TABLE " + db_config.value("table2").toString();
    aDB.exec(querystring);
    if (aDB.lastError().isValid())
    {
        auto test = aDB.lastError().text();
        return false;
    }
    return true;
}

bool deleteRecord(const QSqlDatabase& aDB, const QString& aTable, const QString& aKey, QVariant aValue){
    QSqlQuery query(aDB);
    query.prepare("delete from " + aTable + " where " + aKey + "=?");
    query.addBindValue(aValue);
    return query.exec();
}

void testMySQL(){
    bool test = false;
    QSqlDatabase sql;
    if (tryPrepareTable(sql))
        test = dropTable(sql);
//    test = deleteRecord(db_config.value("table").toString(), "carryid", 1);

    if (tryPrepareTable(sql)){
        test = insertRecord(sql, db_config.value("table").toString(), rea::Json(
            "timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"),  //https://stackoverflow.com/questions/31582537/mysql-timestamp-to-qdatetime-with-milliseconds
            "qrcode", "xxxx1",
            "carryid", 1,
            "inspectinfo", "hello world"));
        test = insertRecord(sql, db_config.value("table").toString(), rea::Json(
            "timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"),  //https://stackoverflow.com/questions/31582537/mysql-timestamp-to-qdatetime-with-milliseconds
            "qrcode", "xxxx2",
            "carryid", 2,
            "inspectinfo", "hello world"));
        test = insertRecord(sql, db_config.value("table").toString(), rea::Json(
            "timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"),  //https://stackoverflow.com/questions/31582537/mysql-timestamp-to-qdatetime-with-milliseconds
            "qrcode", "xxxx3",
            "carryid", 3,
            "inspectinfo", "hello world"));
        test = insertRecord(sql, db_config.value("table").toString(), rea::Json(
              "timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"),  //https://stackoverflow.com/questions/31582537/mysql-timestamp-to-qdatetime-with-milliseconds
              "qrcode", "xxxx4",
              "carryid", 4,
              "inspectinfo", "hello world"));
        auto tmp = accessRecord(sql, db_config.value("table").toString(), "carryid", 1);
    }
}
