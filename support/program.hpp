#ifndef PROGRAM_HPP_39793FD2_7845_47A7_9E21_6DDAD42C9A09
#define PROGRAM_HPP_39793FD2_7845_47A7_9E21_6DDAD42C9A09

#include <glad.h>

#include <string>
#include <vector>

#include <cstdint>
#include <cstdlib>

#include "../main/glm.hpp"

class ShaderProgram final
{
	public:
		struct ShaderSource
		{
			GLenum type;
			std::string sourcePath;
		};

	public:
		explicit ShaderProgram( 
			std::vector<ShaderSource> = {}
		);

		~ShaderProgram();

		ShaderProgram( ShaderProgram const& ) = delete;
		ShaderProgram& operator= (ShaderProgram const&) = delete;

		ShaderProgram( ShaderProgram&& ) noexcept;
		ShaderProgram& operator= (ShaderProgram&&) noexcept;

	public:
		void use();

		GLuint programId() const noexcept;

		void reload();

		void setBool(const std::string& name, bool value) const
		{
			glUniform1i(glGetUniformLocation(mProgram, name.c_str()), (int)value);
		}
		// ------------------------------------------------------------------------
		void setInt(const std::string& name, int value) const
		{
			glUniform1i(glGetUniformLocation(mProgram, name.c_str()), value);
		}
		// ------------------------------------------------------------------------
		void setFloat(const std::string& name, float value) const
		{
			glUniform1f(glGetUniformLocation(mProgram, name.c_str()), value);
		}
		// ------------------------------------------------------------------------
		void setVec2(const std::string& name, const glm::vec2& value) const
		{
			glUniform2fv(glGetUniformLocation(mProgram, name.c_str()), 1, &value[0]);
		}
		void setVec2(const std::string& name, float x, float y) const
		{
			glUniform2f(glGetUniformLocation(mProgram, name.c_str()), x, y);
		}
		// ------------------------------------------------------------------------
		void setVec3(const std::string& name, const glm::vec3& value) const
		{
			glUniform3fv(glGetUniformLocation(mProgram, name.c_str()), 1, &value[0]);
		}
		void setVec3(const std::string& name, float x, float y, float z) const
		{
			glUniform3f(glGetUniformLocation(mProgram, name.c_str()), x, y, z);
		}
		// ------------------------------------------------------------------------
		void setVec4(const std::string& name, const glm::vec4& value) const
		{
			glUniform4fv(glGetUniformLocation(mProgram, name.c_str()), 1, &value[0]);
		}
		void setVec4(const std::string& name, float x, float y, float z, float w)
		{
			glUniform4f(glGetUniformLocation(mProgram, name.c_str()), x, y, z, w);
		}
		// ------------------------------------------------------------------------
		void setMat2(const std::string& name, const glm::mat2& mat) const
		{
			glUniformMatrix2fv(glGetUniformLocation(mProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}
		// ------------------------------------------------------------------------
		void setMat3(const std::string& name, const glm::mat3& mat) const
		{
			glUniformMatrix3fv(glGetUniformLocation(mProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}
		// ------------------------------------------------------------------------
		void setMat4(const std::string& name, const glm::mat4& mat) const
		{
			glUniformMatrix4fv(glGetUniformLocation(mProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}
	private:
		GLuint mProgram;
		std::vector<ShaderSource> mSources;
};

#endif // PROGRAM_HPP_39793FD2_7845_47A7_9E21_6DDAD42C9A09
