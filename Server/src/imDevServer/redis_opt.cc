/*
 * CRedisOpt.cpp
 *
 *  Created on: 2014-2-13
 *      Author: zhangs
 */

#include "redis_opt.h"
#include "../public/utils.h"

#define OK "OK"
#define CH_OFFLINE "0"

CRedisOpt::CRedisOpt()
{
	conn_ = NULL;
	reply_= NULL;
}

CRedisOpt::~CRedisOpt()
{
}

bool CRedisOpt::SelectDB(int db_num)
{
	bool bret = true;
	std::string command = std::string("SELECT ") + utils::int2str(db_num);
	reply_= (redisReply*) redisCommand(conn_, command.c_str());
	if(NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_ERROR == reply_->type)
	{
		bret = false;
		LOG4CXX_ERROR(g_logger, "CRedisOpt::SelectDB errorMessage = " << reply_->str);
	}
	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::Set(const std::string& key, const std::string& value)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "SET  %s %s", key.c_str(), value.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_STATUS == reply_->type && strcmp(reply_->str, OK) == 0)
	{
		bret = true;
	}
	else
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::SET error. set_c_c " << key << ":" << value << ", errorMessage = " << reply_->str);
	}
	SafeFreeReplyObject(reply_);

	return bret;
}

bool CRedisOpt::Set(const std::string& key, const int value)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "SET  %s %d", key.c_str(), value);
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_STATUS == reply_->type && strcmp(reply_->str, OK) == 0)
	{
		bret = true;
	}
	else
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::SET error. set_c_i " << key << ":" << value << ", errorMessage = " << reply_->str);
	}
	SafeFreeReplyObject(reply_);

	return bret;
}

bool CRedisOpt::Set(const std::string& key, const long value)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "SET  %s %ld", key.c_str(), value);
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_STATUS == reply_->type && strcmp(reply_->str, OK) == 0)
	{
		bret = true;
	}
	else
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::SET error. set_c_l " << key << ":" << value << ", errorMessage = " << reply_->str);
	}
	SafeFreeReplyObject(reply_);

	return bret;
}

bool CRedisOpt::Get(const std::string& key, std::string& value)
{
	bool bret = false;
	reply_ = (redisReply*) redisCommand(conn_, "GET  %s", key.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_STRING == reply_->type)
	{
		bret = true;
		value.assign(reply_->str);
	}
	else if (REDIS_REPLY_NIL == reply_->type)
	{
		LOG4CXX_TRACE(g_logger, "CRedisOpt::GET Nil. key = " << key);
	}
	else
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::GET error. key = " << key << ", errorMessage = " << reply_->str);
	}

	SafeFreeReplyObject(reply_);

	return bret;
}

bool CRedisOpt::MGet(const std::vector<std::string>& vec_key, const std::string& extend_keyname, std::vector<std::string>& vec_value)
{
	bool bret = false;
	if (vec_key.empty())
	{
		return bret;
	}

	std::string command = "MGET " + utils::JoinListByDelimiter(vec_key, extend_keyname +std::string(" "));

	reply_ = (redisReply*) redisCommand(conn_, command.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	};

	if(REDIS_REPLY_ARRAY != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::MGet error. command = " << command << ", errorMessage = " << reply_->str);
		SafeFreeReplyObject(reply_);
		return bret;
	}

	bret = true;
	int len = reply_->elements;
	char* temp;
	for (int i = 0; i < len; ++i)
	{
		temp = reply_->element[i]->str;
		if(temp == NULL)
			vec_value.push_back(CH_OFFLINE);
		else
			vec_value.push_back(temp);
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::Incr(const std::string& key)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "INCR  %s ", key.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	};

	if (REDIS_REPLY_INTEGER != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::Incr error.");
	}
	else
	{
		bret = true;
	}
	SafeFreeReplyObject(reply_);

	return bret;
}

bool CRedisOpt::LPush(const std::string& key, const std::vector<std::string>& vec_value)
{
	bool bret = false;
	if (vec_value.empty())
	{
		return bret;
	}

	std::string command = "LPUSH " + key + " " + utils::JoinListByDelimiter(vec_value, " ");

	reply_ = (redisReply*) redisCommand(conn_, command.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_INTEGER != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::LPush error. command = " << command << ", errorMessage = " << reply_->str);
	}
	else
	{
		bret = true;
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::LPush(const std::string& key, const std::string& value)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "LPUSH %s %s", key.c_str(), value.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_INTEGER != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::LPush errorMessage = " << reply_->str);
	}
	else
	{
		bret = true;
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::RPush(const std::string& key, const std::string& value)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "RPUSH %s %s", key.c_str(), value.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_INTEGER != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::RPush errorMessage = " << reply_->str);
	}
	else
	{
		bret = true;
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::LPop(const std::string& key)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "LPOP %s", key.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_ERROR == reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::LPop error. " << key << ", errorMessage = " << reply_->str);
	}
	else
	{
		bret = true;
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::LLen(const std::string& key, int& len)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "LLEN %s", key.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_INTEGER != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::LLen error. " << key  << ", errorMessage = " << reply_->str);
	}
	else
	{
		bret = true;
		len = reply_->type;
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::LRange(const std::string& key, std::vector<std::string>& vec_value)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "LRANGE %s 0 -1", key.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_ARRAY != reply_->type)
	{
		SafeFreeReplyObject(reply_);
		LOG4CXX_ERROR(g_logger, "CRedisOpt::LRange error. key = " << key << ", errorMessage = " << reply_->str);
		return bret;
	}

	bret = true;
	int len = reply_->elements;
	for (int i = 0; i < len; ++i)
	{
		vec_value.push_back(reply_->element[i]->str);
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::SAdd(const std::string& key, const std::vector<std::string>& vec_value)
{
	bool bret = false;

	if(vec_value.empty())
		return bret;

	std::string command = "SADD " + key + " " + utils::JoinListByDelimiter(vec_value, " ");

	reply_ = (redisReply*) redisCommand(conn_, command.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_INTEGER != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::SAdd error. command = " << command << ", errorMessage = " << reply_->str);
	}
	else
	{
		bret = true;
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::SAdd(const std::string& key, const std::string& value)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "SAdd %s %s", key.c_str(), value.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_INTEGER != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::SAdd  errorMessage = " << reply_->str);
	}
	else
	{
		bret = true;
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::SAdd(const std::string& key, const std::set<std::string>& set_value)
{
	bool bret = false;

	if (set_value.empty())
		return bret;

	std::string command = "SADD " + key + " " + utils::JoinSetByDelimiter(set_value, " ");

	reply_ = (redisReply*) redisCommand(conn_, command.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_INTEGER != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::SAdd_s error. command = " << command);
	}
	else
	{
		bret = true;
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::SRem(const std::string& key, const std::string& value)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "SREM %s %s", key.c_str(), value.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_INTEGER != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::SRem  errorMessage = " << reply_->str);
	}
	else
	{
		bret = true;
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::SMembers(const std::string& key, std::vector<std::string>& vec_value)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "SMEMBERS %s ", key.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_ARRAY != reply_->type)
	{
		SafeFreeReplyObject(reply_);
		LOG4CXX_ERROR(g_logger, "CRedisOpt::SMembers error. key = " << key << ", errorMessage = " << reply_->str);
		return bret;
	}

	bret = true;
	int len = reply_->elements;
	for (int i = 0; i < len; ++i)
	{
		vec_value.push_back(reply_->element[i]->str);
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::Hset(const std::string& key, const std::string& field, const int value)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "HSET %s %s %d", key.c_str(), field.c_str(), value);
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_INTEGER != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::Hset_c_i error. " << key << ":" << field << ":" << value << ", errorMessage = " << reply_->str);
	}
	else
	{
		bret = true;
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::Hset(const std::string& key, const std::string& field, const std::string& value)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "HSET %s %s %s", key.c_str(), field.c_str(), value.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_INTEGER != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::Hset_c_c error. " << key << ":" << field << ":" << value << ", errorMessage = " << reply_->str);
	}
	else
	{
		bret = true;
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::Hget(const std::string& key, const std::string& field, std::string& value)
{
	bool bret = true;

	reply_ = (redisReply*) redisCommand(conn_, "HGET %s %s ", key.c_str(), field.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_ERROR == reply_->type)
	{
		bret = false;
		LOG4CXX_ERROR(g_logger, "CRedisOpt::Hget_c_c . " << key << ":" << field << ":" << value << ", errorMessage = " << reply_->str);
	}
	else if (REDIS_REPLY_NIL == reply_->type)
	{
		LOG4CXX_WARN(g_logger, "CRedisOpt::Hget_c_c nil. " << key << ":" << field << ":" << value);
	}
	else
	{
		value.assign(reply_->str);
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::Hdel(const std::string& key, const std::string& field)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "HDEL %s %s", key.c_str(), field.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_INTEGER != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::Hdel error. " << key << ":" << field<< ", errorMessage = " << reply_->str);
	}
	else
	{
		bret = true;
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::Hvals(const std::string& key, std::vector<std::string>& vec_value)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "HVALS %s ", key.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_ARRAY != reply_->type)
	{
		SafeFreeReplyObject(reply_);
		LOG4CXX_ERROR(g_logger, "CRedisOpt::Hvals error. key = " << key<< ", errorMessage = " << reply_->str);
		return bret;
	}

	bret = true;
	int len = reply_->elements;
	for(int i = 0; i < len; ++i)
	{
		vec_value.push_back(reply_->element[i]->str);
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::HLen(const std::string& key, int& len)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "HLEN %s", key.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_INTEGER != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::HLen error. " << key  << ", errorMessage = " << reply_->str);
	}
	else
	{
		bret = true;
		len = reply_->type;
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::Del(const std::string& key)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "DEL %s ", key.c_str());
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_INTEGER != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::Del error. key " << key << ", errorMessage = " << reply_->str);
	}
	else
	{
		bret = true;
	}

	SafeFreeReplyObject(reply_);
	return bret;
}

bool CRedisOpt::Expire(const std::string& key, const int sec)
{
	bool bret = false;

	reply_ = (redisReply*) redisCommand(conn_, "EXPIRE %s %d", key.c_str(), sec);
	if (NULL == reply_)
	{
		LOG4CXX_ERROR(g_logger, "Expire:redisCommand: error = " << conn_->errstr);
		SafeFreeRedisContext(conn_);
		return false;
	}

	if (REDIS_REPLY_INTEGER != reply_->type)
	{
		LOG4CXX_ERROR(g_logger, "CRedisOpt::Expire error. key " << key << ", errorMessage = " << reply_->str);
	}
	else if(1 == reply_->integer)
	{
		bret = true;
	}

	SafeFreeReplyObject(reply_);
	return bret;
}
