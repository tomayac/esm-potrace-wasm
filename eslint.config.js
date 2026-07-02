import google from 'eslint-config-google';
import prettier from 'eslint-config-prettier/flat';

const unsupportedGoogleRules = new Set(['require-jsdoc', 'valid-jsdoc']);
const googleRules = Object.fromEntries(
  Object.entries(google.rules).filter(
    ([rule]) => !unsupportedGoogleRules.has(rule)
  )
);

export default [
  {
    ignores: ['dist/**'],
  },
  {
    languageOptions: {
      ecmaVersion: 2020,
      sourceType: 'module',
    },
    rules: {
      ...googleRules,
      ...prettier.rules,
    },
  },
];
