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
        explicit ParseException(const std::string& msg, const std::string& str = std::string(), int pos = -1) : runtime_error(pos >= 0 && pos < static_cast<int>(str.size()) ? msg + ": " + str.substr(0, pos) + " ---> " + str.substr(pos) : msg), _message(msg), _string(str), _position(pos) { }

        const std::string& getMessage() const {
            return _message;
        }

        const std::string& getString() const {
            return _string;
        }

        int getPosition() const {
            return _position;
        }

    private:
        std::string _message;
        std::string _string;
        int _position;
    };      

    class GenerateException : public std::runtime_error {
    public:
        explicit GenerateException(const std::string& msg) : runtime_error(msg) { }
    };      

    class FileException : public std::runtime_error {
    public:
        explicit FileException(const std::string& msg, const std::string& fileName) : runtime_error(fileName.empty() ? msg : msg + ": " + fileName), _message(msg), _fileName(fileName) { }

        const std::string& getMessage() const {
            return _message;
        }

        const std::string& getFileName() const {
            return _fileName;
        }

    private:
        std::string _message;
        std::string _fileName;
    };

    class NetworkException : public std::runtime_error {
    public:
        explicit NetworkException(const std::string& msg, const std::string& details = std::string()) : runtime_error(details.empty() ? msg : msg + ": " + details), _message(msg), _details(details) { }

        const std::string& getMessage() const {
            return _message;
        }

        const std::string& getDetails() const {
            return _details;
        }

    private:
        std::string _message;
        std::string _details;
    };

    class GenericException : public std::runtime_error {
    public:
        explicit GenericException(const std::string& msg, const std::string& details = std::string()) : runtime_error(details.empty() ? msg : msg + ": " + details), _message(msg), _details(details) { }

        const std::string& getMessage() const {
            return _message;
        }

        const std::string& getDetails() const {
            return _details;
        }

    private:
        std::string _message;
        std::string _details;
    };

}

#endif
