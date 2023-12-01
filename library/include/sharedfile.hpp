#pragma once
#ifndef SHARED_FILE_HPP
#define SHARED_FILE_HPP

#include <memory>
#include <fstream>
#include <vector>

#include <cstdint>

class SharedFile
{
public:
	SharedFile();
	SharedFile(const SharedFile &) = default;
	virtual ~SharedFile();

	/**
	 * @brief Open a file by path
	 * @param path The path of the file
	 * @return True on success, false otherwise
	 */
	virtual bool open(const std::string & path) = 0;

	/**
	 * @brief Open a specific file
	 * @param file The file to open
	 * @return True on success, false otherwise
	 */
	bool openFile(const std::string & file);

	/**
	 * @brief Close the file
	 */
	void close();

	/**
	 * @brief Get the file name
	 * @return The file to read from
	 */
	virtual std::string file() const = 0;

	/**
	 * @brief Get the last error
	 * @return The last error 
	 */
	const std::string & getLastError() const { return _last_error; }

	/**
	 * @brief Read the whole file
	 * @return Storage for file content 
	 */
	std::vector<uint8_t> readAll();

protected:

	virtual bool read(uint8_t * out, std::size_t size);
	bool isOpen() const;
	uint64_t size() const;
	void seek(uint64_t offset);

private:
	std::shared_ptr<std::ifstream> _in;

	std::string _last_error;
};

#endif // SHARED_FILE_HPP
