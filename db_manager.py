import pymysql
import json
import os

class DatabaseManager:
    def __init__(self):
        self.db_active = False
        try:
            self.config = self._load_config()
            self._ensure_db_and_table()
            self.db_active = True
            print("[DATABASE]: Connected successfully.")
        except Exception as e:
            print(f"[DATABASE ERROR]: Database is unavailable ({e}). The app will work in offline mode.")
    
    def _load_config(self):
        config_path = "db_config.json"

        if not os.path.exists(config_path):
            raise FileExistsError(f"configuration file {config_path} wasn't found")
        
        with open(config_path, "r", encoding="utf-8") as f:
            return json.load(f)
    
    def _ensure_db_and_table(self):
        temp_conn = pymysql.connect(
            host=self.config['host'],
            user=self.config['user'],
            password=self.config['password']
        )
        try:
            with temp_conn.cursor() as cursor:
                cursor.execute(f"CREATE DATABASE IF NOT EXISTS {self.config['database']}")
            temp_conn.commit()
        finally:
            temp_conn.close()

        connection = pymysql.connect(**self.config)
        try:
            with connection.cursor() as cursor:
                cursor.execute("""
                    CREATE TABLE IF NOT EXISTS encryption_results (
                        id INT AUTO_INCREMENT PRIMARY KEY,
                        algorithm_name VARCHAR(255),
                        message TEXT,
                        encrypted_message TEXT,
                        execution_time FLOAT,
                        difficulty VARCHAR(15),
                        memory_usage FLOAT,
                        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                    )
                """)
                connection.commit()
        finally:
            connection.close()
    
    def save_result(self, algo_name, msg, enc_msg, exec_time, diff, mem):
        if not self.db_active:
            print(f"[DATABASE SKIP]: Result for {algo_name} not saved (DB offline).")
            return

        try:
            connection = pymysql.connect(**self.config)
            with connection.cursor() as cursor:
                sql = """
                    INSERT INTO encryption_results 
                    (algorithm_name, message, encrypted_message, execution_time, difficulty, memory_usage)
                    VALUES (%s, %s, %s, %s, %s, %s)
                """
                cursor.execute(sql, (algo_name, msg, enc_msg, exec_time, diff, mem))
            connection.commit()
        except Exception as e:
            print(f"Database Error: {e}")
        finally:
            connection.close()