#ifndef SCANNER_H
#define SCANNER_H

#include <vector>
#include <string>
#include <optional>
#include <functional>

#include "common.h"

template <typename T>
class Scanner {
	using TOp = std::optional<T>;
	using ScanCond = std::function<bool(T)>;
public:
	Scanner() = default;
	~Scanner() = default;

	Scanner(const std::vector<T>& input) : m_data(input) {}

	bool hasNext() const { return !m_data.empty(); }
	size_t len() const { return m_data.size(); }

	TOp scan() {
		if (!hasNext()) return {};
		T data = m_data.front();
		m_data.erase(m_data.begin());
		return data;
	}

	TOp peek() {
		if (!hasNext()) return {};
		return m_data[0];
	}

	std::vector<T> scanWhile(const ScanCond& cond) {
		std::vector<T> ret;
		while (hasNext() && cond(m_data[0])) {
			ret.push_back(scan().value());
		}
		return ret;
	}

protected:
	std::vector<T> m_data;
};

class StringScanner : public Scanner<char> {
	using ScanCond = std::function<bool(char)>;
public:
	StringScanner() = default;
	~StringScanner() = default;

	StringScanner(const std::string& value)
		: Scanner(std::vector<char>(value.begin(), value.end()))
	{}

	std::string scanString(const ScanCond& cond) {
		std::vector<char> res = scanWhile(cond);
		return std::string(res.begin(), res.end());
	}

};

#endif // SCANNER_H