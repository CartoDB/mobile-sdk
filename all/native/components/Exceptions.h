/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_EXCEPTIONS_H_
#define _CARTO_EXCEPTIONS_H_

#include <stdexcept>
#include <string>

namespace carto {

    class NullArgumentException : public std::invalid_argument {
    public:
        explicit NullArgumentException(const std::string& msg) : invalid_argument(msg) { }
    };

    class InvalidArgumentException : public std::invalid_argument {
    public:
        explicit InvalidArgumentException(const std::string& msg) : invalid_argument(msg) { }
    };

    class OutOfRangeException : public std::out_of_range {
    public:
        explicit OutOfRangeException(const std::string& msg) : out_of_range(msg) { }
    };

    class ParseException : public std::runtime_error {
    public:
        explicit ParseException(const std::string& msg, const std::string& str = std::string(), int position = -1) : runtime_error(msg), _string(str), _position(position) { }

        const std::string& getString() const {
            return _string;
        }

        int getPosition() const {
            return _position;
        }

    private:
        std::string _string;
        int _position;
    };      

    class GenerateException : public std::runtime_error {
    public:
        explicit GenerateException(const std::string& msg) : runtime_error(msg) { }
    };      

    class FileException : public std::runtime_error {
    public:
        explicit FileException(const std::string& msg, const std::string& fileName) : runtime_error(msg), _fileName(fileName) { }

        const std::string& getFileName() const {
            return _fileName;
        }

    private:
        std::string _fileName;
    };

    class NetworkException : public std::runtime_error {
    public:
        explicit NetworkException(const std::string& msg, const std::string& details = std::string()) : runtime_error(msg), _details(details) { }

        const std::string& getDetails() const {
            return _details;
        }

    private:
        std::string _details;
    };

    class GenericException : public std::runtime_error {
    public:
        explicit GenericException(const std::string& msg, const std::string& details = std::string()) : runtime_error(msg), _details(details) { }

        const std::string& getDetails() const {
            return _details;
        }

    private:
        std::string _details;
    };

}

#endif
